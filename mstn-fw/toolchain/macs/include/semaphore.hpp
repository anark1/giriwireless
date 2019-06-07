/** @copyright AstroSoft Ltd */
#pragma once

#include "scheduler.hpp" 

namespace macs
{
 
class Semaphore: public SyncObject
{
public:
	Semaphore(size_t start_count = 0, size_t max_count = 1);
	~Semaphore();
	 
	size_t GetCurrentCount() const
	{
		return m_count;
	}
	 
	size_t GetMaxCount() const
	{
		return m_max_count;
	}

	Result Wait(uint32_t timeout_ms = INFINITE_TIMEOUT);
	Result Signal();
	static Result Wait_Priv(Semaphore * pS, uint32_t timeout_ms);  
	static Result Signal_Priv(Semaphore * pS);  

private:
	CLS_COPY(Semaphore)

	bool TryDecrement()
	{
		return m_count ? (--m_count, true) : false;
	}

private:
	size_t m_count;
	size_t m_max_count;
};

 
class BinarySemaphore: public Semaphore
{
public:
	 
	explicit BinarySemaphore(bool is_empty = true) :
			Semaphore(is_empty ? 0 : 1, 1)
	{
	}
};

}  
