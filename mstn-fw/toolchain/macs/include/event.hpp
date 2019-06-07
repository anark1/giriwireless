/** @copyright AstroSoft Ltd */
#pragma once

#include "scheduler.hpp"

namespace macs
{
 
class Event: public SyncObject
{
public:
	Event(bool broadcast = true);
	~Event();

	bool GetBroadcast()
	{
		return m_broadcast;
	}
	 
	Result Wait(uint32_t timeout_ms = INFINITE_TIMEOUT);
	Result Raise();
	static Result Wait_Priv(Event* pE, uint32_t timeout_ms);  
	static Result Raise_Priv(Event* pE);  

private:
	CLS_COPY(Event)

private:
	bool m_broadcast;
};

}  
