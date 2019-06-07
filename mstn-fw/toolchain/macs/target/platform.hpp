/** @copyright AstroSoft Ltd */
#pragma once

#include <stdint.h>
#include "common.hpp"

#define MACS_CORTEX_M0		1
#define MACS_CORTEX_M0_P	5
#define MACS_CORTEX_M1		10
#define MACS_CORTEX_M3		30
#define MACS_CORTEX_M4		40

#if defined(STM32F429xx)
#define MACS_MCU_CORE  MACS_CORTEX_M4
#define MACS_PLATFORM_INCLUDE_1  "stm32f4xx.h"
#define MACS_PLATFORM_INCLUDE_2  "stm32f4xx_hal.h"
#endif	

#if defined(MDR1986VE9x)
#define MACS_MCU_CORE  MACS_CORTEX_M3
#define MACS_PLATFORM_INCLUDE_1  "MDR32Fx.h"
#endif

#ifdef MDR1986VE1T
#define MACS_MCU_CORE  MACS_CORTEX_M1
#define MACS_PLATFORM_INCLUDE_1  "MDR1986VE1T.h"
#endif	

#include "stack_frame.hpp"

#if MACS_USE_MPU
typedef enum
{
	MPU_ZERO_ADR_MINE = 1,
	MPU_PROC_STACK_MINE,
	MPU_MAIN_STACK_MINE
}MPU_MINE_NUM;

extern void MPU_Init();
extern void MPU_SetMine(MPU_MINE_NUM, uint32_t adr);
extern void MPU_RemoveMine(MPU_MINE_NUM);
#endif		

class StackPtr
{
private:
	static const uint32_t TOP_MARKER = 0xA52E3FC1;
public:
	enum CHECK_RES
	{
		SP_OK = 0,
		SP_OVERFLOW,
		SP_UNDERFLOW,
		SP_CORRUPTED
	};
	uint32_t * m_sp;
public:
	StackPtr(uint32_t * sp = nullptr)
	{
		Set(sp);
	}
	inline void Set(uint32_t * sp)
	{
		m_sp = sp;
	}
	inline void Zero()
	{
		Set(nullptr);
	}
	size_t GetVirginLen(StackPtr marg) const;
	CHECK_RES Check(StackPtr marg, size_t len);
	void Instrument(StackPtr marg, bool do_full);
#if MACS_MPU_PROTECT_STACK		
	inline void SetMpuMine() {
		MPU_SetMine(MPU_PROC_STACK_MINE, ((uint32_t) m_sp & ~0x1F) - 0x20);
	}
#endif
private:
	static void FillWithMark(uint32_t * ptr, uint32_t * lim);
	static size_t GetVirginLen(uint32_t * beg, uint32_t * lim);
};

class TaskStack
{
private:
	static const size_t WORK_SIZE = 0x10;
public:
	static const size_t MIN_SIZE
#if (__FPU_USED == 1)
	= 0x34 + WORK_SIZE;
#else
	= 0x12 + WORK_SIZE;
#endif
	static const size_t ENOUGH_SIZE
#if MACS_MCU_CORE == MACS_CORTEX_M3
	= 500;
#else
	= 350;
#endif
private:
#if MACS_MPU_PROTECT_STACK		
	static const size_t GUARD_SIZE = MAX(WORK_SIZE, (2 * 32/4) - 1);
#else
	static const size_t GUARD_SIZE = WORK_SIZE;
#endif
	static const size_t MIN_REST = 2 * WORK_SIZE;
	static const int GROW_SIZE = MIN_SIZE;
public:
	static const size_t MAX_SIZE = MACS_MAX_STACK_SIZE - GUARD_SIZE;
private:
	bool m_is_alien_mem;
	size_t m_len;
	uint32_t * m_memory;
	StackPtr m_margin;
	void PreparePlatformSpecific(size_t len, void * this_ptr, void (*run_func)(void), void (*exit_func)(void));
public:
	StackPtr m_top;
public:
	TaskStack()
	{
		m_is_alien_mem = false;
		m_len = 0;
		m_memory = nullptr;
	}
	~TaskStack()
	{
		Free();
	}
	void Build(size_t len, uint32_t * mem = nullptr);
	void BuildPlatformSpecific(size_t guard, size_t len);
	void Prepare(size_t len, void * this_ptr, void (*run_func)(void), void (*exit_func)(void));
	void Free();
	void Instrument()
	{
		m_top.Instrument(m_margin, true);
	}
	inline size_t GetLen() const
	{
		return m_len;
	}
	inline size_t GetUsage() const
	{
		return m_len - m_top.GetVirginLen(m_margin);
	}
	bool Check();
#if MACS_MPU_PROTECT_STACK		
	inline void SetMpuMine() {m_margin.SetMpuMine();}
#endif
};

class SystemBase
{
private:
	static uint32_t m_tick_rate_hz;
	static const uint m_stack_alignment;
public:
	static const uint32_t INITIAL_XPSR = 0x01000000;
	static const int MAX_SYSCALL_INTERRUPT_PRIORITY = 5;
	static const int FIRST_USER_INTERRUPT_NUMBER = 16;
	static const uint32_t INTERRUPT_MIN_PRIORITY = 0xFFu;
	static const uint32_t CONTROL_UNPRIV_FLAG = 0x01;
#if MACS_MPU_PROTECT_STACK
	static const size_t MacsMainStackSize = MACS_MAIN_STACK_SIZE;
	static uint32_t * MacsMainStackBottom;
#endif

public:

	static uint32_t DisableIrq();
	static void EnableIrq(uint32_t mask);
	static void SetIrqPriority(int irq_num, uint priority);
	static int CurIrqNum();
	static bool IsInSysCall();
	static bool inline SetUpIrqHandling(int irq_num, bool vector, bool enable)
	{
		return false;
	}

	static bool IsInInterrupt();
	static bool IsInPrivMode();
	static inline bool IsInPrivOrIrq()
	{
		return IsInPrivMode() || IsInInterrupt();
	}

	static bool IsSysCallAllowed();
	static inline uint GetStackAlignment()
	{
		return m_stack_alignment;
	}

	static bool IsInMspMode();
	static uint32_t GetMsp();
	static void SetPsp(StackPtr sp);
	static void FirstSwitchToTask(StackPtr sp, bool is_privileged);
	static void SetPrivMode(bool is_on);
	static void SwitchContext();
	static bool InitScheduler();

#if MACS_USE_MPU
	static void MpuInit ();
	static void MpuSetMine (uint32_t rnum, uint32_t adr);
	static void MpuRemoveMine (uint32_t rnum);
#endif
	static inline uint32_t GetCpuFreq()
	{
		return SystemCoreClock;
	}

	static inline uint32_t GetTickRate()
	{
		return m_tick_rate_hz;
	}

	static inline float GetTickPeriod()
	{
		return ((float)1000) / m_tick_rate_hz;
	}

	static bool SetTickRate(uint32_t rate_hz);

	static inline bool SetTickPeriod(float inPeriod)
	{
		return SetTickRate(1000 / inPeriod);
	}

	static ulong GetCurCpuTick();
	static void SetCurCpuTick(ulong tk);
	static ulong AskCurCpuTick();

	static inline ulong CpuTicksToNs(ulong cpu_ticks)
	{
		_ASSERT(cpu_ticks <= ULONG_MAX / 1000);
		return (1000 * cpu_ticks) / (GetCpuFreq() / 1000000);
	}
	static inline ulong CpuTicksToUs(ulong cpu_ticks)
	{
		return cpu_ticks / (GetCpuFreq() / 1000000);
	}
	static inline ulong CpuToOsTicks(ulong cpu_ticks)
	{
		return cpu_ticks / (GetCpuFreq() / GetTickRate());
	}

	static inline ulong ReadUs()
	{
		return CpuTicksToUs(GetCurCpuTick());
	}
	static inline ulong ReadMs()
	{
		return ReadUs() / 1000;
	}

	static inline void WaitNs(ulong delay)
	{
		delay = ((GetCpuFreq() / 1000000) * delay) / 1000;
		ulong start_tick = GetCurCpuTick();
		while (GetCurCpuTick() - start_tick < delay)
			;
	}
	static inline void WaitUs(ulong delay_us)
	{
		_ASSERT(delay_us <= ULONG_MAX / 1000);
		WaitNs(1000 * delay_us);
	}
	static inline void WaitMs(ulong delay_ms)
	{
		_ASSERT(delay_ms <= ULONG_MAX / 1000000);
		WaitNs(1000000 * delay_ms);
	}

	static void McuReset();

	static inline void Crash(ALARM_REASON)
	{
		KERNEL_BKPT(1);
	}

	static void InternalSwitchContext();

	static void EnterSleepMode();
};
