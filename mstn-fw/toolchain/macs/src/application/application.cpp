/** @copyright AstroSoft Ltd */

#include <stdio.h>
#include "tunes.h"
#include "application.hpp"
#include "system.hpp"
#include "scheduler.hpp"

Application *Application::m_app = nullptr;

Application::Application(bool use_preemption) :
		m_use_preemption(use_preemption)
{
	_ASSERT(!m_app);
	m_app = this;
}

Application::~Application()
{
}

void Application::Run()
{
	System::InitCpu();

	if (Sch().Initialize() != ResultOk)
		return;

	Initialize();

#if MACS_PRINTF_ALLOWED	
	printf("Приложение запущено.\r\n");
#endif	
	Sch().Start(m_use_preemption);
}

ALARM_ACTION Application::OnAlarm(ALARM_REASON reason)
{
	if (reason != AR_NONE && reason != AR_STACK_ENLARGED)
		MACS_CRASH(reason);
	return AA_CONTINUE;
}

extern "C"
{

void MacsInit()
{
	static bool is_ready = false;
	if (!is_ready) {
		_ASSERT(System::IsInPrivOrIrq());
#if MACS_USE_MPU && MACS_MPU_PROTECT_STACK
		System::MacsMainStackBottom = (uint32_t *)System::GetMsp();
#endif
		is_ready = true;
	}
}

uint32_t HAL_GetTick()
{
	return Sch().GetTickCount();
}

void Hard_Fault_Handler_C(uint32_t stack[])
{
	static volatile uint32_t r0 = stack[0];
	static volatile uint32_t r1 = stack[1];
	static volatile uint32_t r2 = stack[2];
	static volatile uint32_t r3 = stack[3];
	static volatile uint32_t r12 = stack[4];
	static volatile uint32_t lr = stack[5];
	static volatile uint32_t pc = stack[6];
	static volatile uint32_t psr = stack[7];

	System::HardFaultHandler();

#if ! MACS_DEBUG
#if MACS_PRINTF_ALLOWED
	printf("\r\n[Hard fault handler]\r\n");

	printf("R0  = 0x%x\r\n", r0);
	printf("R1  = 0x%x\r\n", r1);
	printf("R2  = 0x%x\r\n", r2);
	printf("R3  = 0x%x\r\n", r3);
	printf("R12 = 0x%x\r\n", r12);
	printf("LR  = 0x%x\r\n", lr);
	printf("PC  = 0x%x\r\n", pc);
	printf("PSR = 0x%x\r\n", psr);
#endif

	App().OnAlarm(AR_HARD_FAULT);
#else
	KERNEL_BKPT(AR_HARD_FAULT);
#endif	
}

void NMI_Handler()
{
#if ! MACS_DEBUG	
	App().OnAlarm(AR_NMI_RAISED);
	for (;;)
		;
#else	
	KERNEL_BKPT(AR_NMI_RAISED);
#endif
}

void MemManage_Handler_C(uint32_t stack[])
{
	static volatile uint32_t source_adr = stack[5];

#if ! MACS_DEBUG	
	ALARM_ACTION act = App().OnAlarm(AR_MEMORY_FAULT);
	if (act != AA_CONTINUE)
		MACS_CRASH(AR_MEMORY_FAULT);
#else
	KERNEL_BKPT(AR_MEMORY_FAULT);
#endif
}

}  
