/** @copyright AstroSoft Ltd */
#pragma once

#include "MDR1986VE1T.h"

#include "platform.hpp"
#include "MDR32F9Qx_port.h"
#include "MDR32F9Qx_uart.h"
#include "MDR32F9Qx_rst_clk.h"

#ifndef MAKS_HEAP_SIZE
#define MAKS_HEAP_SIZE (16384)
#endif	

class System: public SystemBase
{
public:
	static const uint32_t HEAP_SIZE = MAKS_HEAP_SIZE;

	static void InitCpu();
	static void HardFaultHandler();
};
