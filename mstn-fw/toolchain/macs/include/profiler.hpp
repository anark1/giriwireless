/** @copyright AstroSoft Ltd */
#pragma once

#include "tunes.h"

#if MACS_PROFILING_ENABLED 

#include <math.h>
#include "common.hpp"

namespace performance
{

typedef enum
{  
	PE_EMPTY_CALL = 0,
	PE_EMPTY_CONSTR,
	PE_EMBRACE,

	PE_ZERO_CALL_A1,
	PE_ZERO_CALL_B1,
	PE_ZERO_CONSTR_A1,
	PE_ZERO_CONSTR_B1,
	PE_ZERO_CONSTR_B1_2,
	PE_ZERO_CONSTR_C1,
	PE_ZERO_CONSTR_C1_2,
	PE_ZERO_CONSTR_D1_2,
	PE_ZERO_CONSTR_D1_2_3,

	PE_INCR_INT,

	PE_CRIT_SEC_INT_ENTR,
	PE_CRIT_SEC_INT_EXIT,
	PE_CRIT_SEC_EXT_ENTR,
	PE_CRIT_SEC_EXT_EXIT,

	PE_MEM_ALLOC,
	PE_MEM_FREE,

	PE_TASK_INIT,
	PE_TASK_ADD,
	PE_TASK_DEL,

	PE_IQR_HANDLE,

	PE_EVENT_INIT,
	PE_EVENT_RAISE,
	PE_EVENT_ACTION,

	PE_MUTEX_INIT,
	PE_MUTEX_LOCK,
	PE_MUTEX_UNLOCK,
	PE_MUTEX_ACTION,

	PE_SEMPH_INIT,
	PE_SEMPH_GIVE,
	PE_SEMPH_TAKE,
	PE_SEMPH_ACTION,

	PE_DELAY_10MS,

	PE_USER_1,
	PE_USER_2,
	PE_USER_3,

	PE_QTTY
} PROF_EYE;

class ProfEye
{
public:
	 
	ProfEye(PROF_EYE eye, bool run);
	~ProfEye();

	inline PROF_EYE Eye() const
	{
		return m_eye;
	}

	void Start();
	void Stop(bool call = true);
	 
	void Kill()
	{
		m_run = false;
	}

	static void Tune();
	 
	void Print(String & str, bool brief = false, bool use_ns = false);
	 
	static void PrintResults(String & str, bool brief = false, bool use_ns = false);

private:
	bool m_run;
	PROF_EYE m_eye;
	tick_t m_start;
	long m_lost;
	ProfEye * m_up_eye;
	static ProfEye * m_cur_eye;
};

class ProfData
{
private:
	bool m_lock;
	long m_time;  
	long m_lost;
	uint64_t m_sqrs;
	long m_min, m_max;
	ulong m_cnt;

	static tick_t m_empty_call_overhead;
	static tick_t m_empty_constr_overhead;
	static tick_t m_embrace_overhead;
	static const int ADJUSTMENT = 19;
public:
	ProfData()
	{
		m_lock = false;
		Clear();
	}

	inline void Clear()
	{
		m_lost = m_max = m_time = 0;
		m_sqrs = 0;
		m_min = LONG_MAX;
		m_cnt = 0;
	}

	inline ulong Count() const
	{
		return m_cnt;
	}

	inline long TimeTot() const
	{
		return m_time + m_lost;
	}

	inline long TimeNet() const
	{
		return m_time;
	}
	 
	inline long TimeOvh() const
	{
		return m_lost;
	}
	 
	inline long TimeAvg() const
	{
		return m_cnt ? TimeNet() / m_cnt : 0;
	}
	 
	inline long TimeMin() const
	{
		return m_cnt ? m_min : 0;
	}
	 
	inline long TimeMax() const
	{
		return m_cnt ? m_max : 0;
	}
	 
	inline ulong TimeDev() const
	{
		return m_cnt ? sqrt((double)(m_sqrs / m_cnt - TimeAvg() * (int64_t)TimeAvg())) : 0;
	}
	 
	void Print(String & str, bool brief = false, bool use_ns = false);

private:
	void Lock(bool set)
	{
		_ASSERT(set != m_lock);
		m_lock = set;
	}

	friend class ProfEye;
};
extern ProfData g_prof_data[PE_QTTY];

}  

using namespace performance;
 
#define PROF_EYE(eye, name) ProfEye pe_##name(eye, true)
#define PROF_DECL(eye, name) ProfEye name(eye, false);
#define PROF_START(name) name.Start();
#define PROF_STOP(name) name.Stop();

#else

#define PROF_EYE(eye, name)
#define PROF_DECL(eye, name)
#define PROF_START(name)
#define PROF_STOP(name)

#endif	 
