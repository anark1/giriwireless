/** @copyright AstroSoft Ltd */
#pragma once

#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>
#include "tunes.h"
#include "nullptr.h"
#include "utils.hpp"
#include "_kernel_bkpt.h"

namespace macs
{
#define CLS_COPY(cls_name) cls_name(const cls_name &); cls_name & operator = (const cls_name &);

enum EPrivilegedMethods
{
	EPM_Read_Cpu_Tick,
	EPM_BlockCurrentTask_Priv,
	EPM_AddTask_Priv,
	EPM_AddTaskIrq_Priv,
	EPM_Yield_Priv,
	EPM_DeleteTask_Priv,
	EPM_UnblockTask_Priv,
	EPM_SetTaskPriority_Priv,
	EPM_Event_Raise_Priv,
	EPM_Event_Wait_Priv,
	EPM_Mutex_Lock_Priv,
	EPM_Mutex_Unlock_Priv,
	EPM_Semaphore_Wait_Priv,
	EPM_Semaphore_Signal_Priv,
	EPM_SpiTransferCore_Initialize_Priv,
	EPM_Spi_PowerControl_Priv,
	EPM_Count  
};

#if MACS_DEBUG
extern void _Assert(bool);
#define _ASSERT(e) _Assert(e)
#else
#define _ASSERT(e)
#endif

#define RET_ERROR(res, ret) { if ( ! (res) ) return (ret); }
#define RET_ASSERT(res, ret) { _ASSERT(res); RET_ERROR(res, ret); }
 
typedef enum
{
	AR_NONE = 0,
	AR_NMI_RAISED,  
	AR_HARD_FAULT,  
	AR_MEMORY_FAULT,  
	AR_NOT_IN_PRIVILEGED,  
	AR_BAD_SVC_NUMBER,  
	AR_COUNTER_OVERFLOW,  
	AR_STACK_CORRUPTED,  
	AR_STACK_OVERFLOW,  
	AR_STACK_UNDERFLOW,  
	AR_SCHED_NOT_ON_PAUSE,  
	AR_MEM_LOCKED,  
	AR_USER_REQUEST,  
	AR_ASSERT_FAILED,  
	AR_STACK_ENLARGED,  
	AR_OUT_OF_MEMORY,  
	AR_SPRINTF_TRUNC,  
	AR_DOUBLE_PRN_FMT,  
	AR_NESTED_MUTEX_LOCK,  
	AR_OWNED_MUTEX_DESTR,  
	AR_BLOCKING_MUTEX_DESTR,  
	AR_PRIV_TASK_ADDING,
	AR_NO_GRAPH_GUARD,  
	AR_UNKNOWN  
} ALARM_REASON;

 
typedef enum
{
	AA_CONTINUE,  
	AA_RESTART_TASK,  
	AA_KILL_TASK,  
	AA_CRASH  
} ALARM_ACTION;

extern uint8_t ExclSet(uint8_t & flag);  
extern uint8_t ExclIncCnt(uint8_t & cnt);  
extern ulong ExclIncCnt(ulong & cnt);  
extern long ExclChg(long & val, long chg);  
extern void * ExclSetPtr(void * & ptr, void * new_val);  

 
typedef uint32_t tick_t;
extern "C" tick_t MacsGetTickCount();  
extern "C" void MacsDelay(ulong ticks);  
extern "C" void MacsCpuDelay(ulong ticks);  

template <typename T>
inline void Swap(T & v1, T & v2)
{
	T tmp = v1;
	v1 = v2;
	v2 = tmp;
}

class SetFlagTemp
{
private:
	bool & m_flag;
	bool m_save;
public:
	SetFlagTemp(bool & flag, bool val) :
			m_flag(flag)
	{
		m_save = flag;
		flag = val;
	}
	~SetFlagTemp()
	{
		m_flag = m_save;
	}
};
#define SET_FLAG_TEMP(flag, val) SetFlagTemp sft_##flag(flag, val)

template <typename bm_enum>
class BitMask
{
private:
	bm_enum m_val;
public:
	BitMask()
	{
		Zero();
	}
	BitMask(bm_enum val)
	{
		m_val = val;
	}
	bm_enum Val() const
	{
		return m_val;
	}
	void Zero()
	{
		m_val = (bm_enum)0;
	}
	bool Check(bm_enum val) const
	{
		return CheckAll(val);
	}
	bool CheckAny(int val) const
	{
		return m_val & val;
	}
	bool CheckAll(int val) const
	{
		return (m_val & val) == val;
	}
	void Set(int val)
	{
		m_val = (bm_enum)val;
	}
	void Add(int val)
	{
		m_val = (bm_enum)(m_val | val);
	}
	void Rem(int val)
	{
		m_val = (bm_enum)(((uint)m_val) & (~(uint)val));
	}
};

typedef uint BitArrCell;
class BitArr
{
private:
	BitArrCell * m_arr;
protected:
	uint m_qty;
public:
	BitArr()
	{
		Init();
	}
	BitArr(uint qty)
	{
		Init();
		Alloc(qty);
	}
	~BitArr()
	{
		Free();
	}
	void Alloc(uint qty)
	{
		Free();
		m_qty = qty;
		m_arr = new BitArrCell[Cells(qty)];
		memset(m_arr, '\0', Cells(qty));
	}
	bool Check(uint ind)
	{
		_ASSERT(ind < m_qty);
		return !!(m_arr[ind / BitsPerCell()] & CellMask(ind));
	}
	void Set(uint ind, bool val)
	{
		_ASSERT(ind < m_qty);
		val ? m_arr[ind / BitsPerCell()] |= CellMask(ind) : m_arr[ind / BitsPerCell()] &= ~CellMask(ind);
	}
private:
	CLS_COPY(BitArr)
	void Init()
	{
		m_qty = 0;
		m_arr = nullptr;
	}
	void Free()
	{
		if (m_arr)
			delete[] m_arr;
		Init();
	}
	static uint BitsPerCell()
	{
		return 8 * sizeof(BitArrCell);
	}
	static uint Cells(uint qty)
	{
		return (qty + (BitsPerCell() - 1)) / BitsPerCell();
	}
	static BitArrCell CellMask(uint ind)
	{
		return 1 << (ind % BitsPerCell());
	}
};

class BitArr2: public BitArr
{
private:
	uint m_width;
public:
	BitArr2()
	{
		m_width = 0;
	}
	BitArr2(uint x_size, uint y_size)
	{
		Alloc(x_size, y_size);
	}
	void Alloc(uint x_size, uint y_size)
	{
		BitArr::Alloc(x_size * y_size);
		m_width = x_size;
	}
	bool Check(uint x_ind, uint y_ind)
	{
		_ASSERT(x_ind < m_width);
		return BitArr::Check(BitInd(x_ind, y_ind));
	}
	void Set(uint x_ind, uint y_ind, bool val)
	{
		_ASSERT(x_ind < m_width);
		BitArr::Set(BitInd(x_ind, y_ind), val);
	}
private:
	CLS_COPY(BitArr2)
	uint BitInd(uint x_ind, uint y_ind)
	{
		return x_ind * m_width + y_ind;
	}
};

extern int RandN(int n);  
inline int RandMM(int min_val, int max_val)
{
	return min_val + (RandN((max_val - min_val) + 1) - 1);
}  
inline bool RandCoin()
{
	return RandN(2) == 1;
}

class PrnFmt
{
private:
	static const int SPRINTF_BUFSZ = 128;
	static char * m_buf;
public:
	PrnFmt(CSPTR format, ...);
	~PrnFmt();
	operator CSPTR()
	{
		return m_buf;
	}
};
extern void Sprintf(char * buf, size_t bufsz, CSPTR format, ...);

extern CSPTR const g_zstr;
inline CSPTR ZSTR(CSPTR str)
{
	return str ? str : g_zstr;
}
class String
{
public:
	static CSPTR const NEWLINE;
private:
	char * m_str;
public:
	String(CSPTR str = nullptr, int len = -1)
	{
		m_str = nullptr;
		if (str) {
			if (len == -1)
				len = strlen(str);
			else
				_ASSERT(len >= 0 && len <= strlen(str));
			Add(str, len);
		}
	}
	~String()
	{
		Clear();
	}
	String & operator =(const String & str)
	{
		return (*this) = (CSPTR)str;
	}
	String & operator =(CSPTR str)
	{
		Clear();
		Add(str);
		return *this;
	}
	bool operator !() const
	{
		return !m_str;
	}
	inline size_t Len() const
	{
		return m_str ? strlen(m_str) : 0;
	}
	inline String & Clear()
	{
		if (m_str)
			delete[] m_str;
		m_str = nullptr;
		return *this;
	}
	inline String & Add(CSPTR str)
	{
		if (str)
			Add(str, strlen(str));
		return *this;
	}
	inline String & Add(char c)
	{
		char buf[2];
		buf[0] = c;
		buf[1] = '\0';
		return Add(buf);
	}
	inline String & NewLine()
	{
		return Add(NEWLINE);
	}
	inline String & operator <<(CSPTR str)
	{
		return Add(str);
	}
	inline String & operator <<(char c)
	{
		return Add(c);
	}
	String & Add(CSPTR ptr, size_t len);
	inline operator CSPTR() const
	{
		return m_str;
	}
	CSPTR Z() const
	{
		return ZSTR((CSPTR)*this);
	}
	inline bool operator ==(CSPTR str) const
	{
		if (!m_str)
			return !str;
		if (!str)
			return !m_str;
		return !strcmp(m_str, str);
	}
	inline int FindAnyChr(CSPTR chrs)
	{
		if (!m_str || !chrs)
			return -1;
		CSPTR p = strpbrk(m_str, chrs);
		return p ? (p - m_str) : -1;
	}
};



extern "C" void MacsInit();

static const int FIRST_VIRT_IRQ = 0x0100;  

const uint32_t INFINITE_TIMEOUT = UINT_MAX;

 
uint32_t MsToTicks(uint32_t ms);

 
uint32_t TicksToUs(uint32_t ticks);

 
 
 
enum Result
{
	ResultOk,
	ResultTimeout,
	ResultErrorInterruptNotSupported,
	ResultErrorSysCallNotAllowed,
	ResultErrorNotSupported,
	ResultErrorInvalidArgs,
	ResultErrorInvalidState  
};

extern "C" CSPTR GetResultStr(Result, bool brief = false);
extern "C" Result SvcExecPrivileged(void* vR0, void* vR1, void* vR2, uint32_t vR3);

}  

using namespace macs;

extern "C"
{
extern uint32_t SystemCoreClock;
}

#define MACS_CRASH(reason)  System::Crash(reason)
