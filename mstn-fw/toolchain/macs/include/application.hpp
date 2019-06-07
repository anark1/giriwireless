/** @copyright AstroSoft Ltd */
#pragma once

#include "common.hpp"
 
namespace macs
{

class Application
{
public:
	virtual ~Application();
	void Run();
	virtual ALARM_ACTION OnAlarm(ALARM_REASON reason);

protected:
	 
	Application(bool use_preemption = true);

private:
	CLS_COPY(Application)
	 
	virtual void Initialize()
	{
	}

public:
	static Application *m_app;

private:
	bool m_use_preemption;
};

inline Application& App()
{
	return *Application::m_app;
}

}
