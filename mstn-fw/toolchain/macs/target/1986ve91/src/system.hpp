/** @copyright AstroSoft Ltd */
#pragma once

#include "platform.hpp"
#include "MDR32Fx.h"

#ifndef MACS_HEAP_SIZE
#define MACS_HEAP_SIZE 16384
#endif	

#ifndef MDR1986VE91VE94
#define MDR1986VE91VE94 1
#endif
class System: public SystemBase
{
public:
	static const uint32_t HEAP_SIZE = MACS_HEAP_SIZE;

	static void InitCpu();
	static void HardFaultHandler();
};
