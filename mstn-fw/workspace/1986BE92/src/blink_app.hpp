#pragma once

#include <stdint.h>
#include "application.hpp"
#include "led.hpp"

class BlinkApp: public Application
{
private:
	virtual void Initialize();
};
