/** @copyright AstroSoft Ltd */
#pragma once

#include "common.hpp"
#include "platform.hpp"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

#define MACS_INIT_CPU_FREQ_MHZ  168 

#ifndef MACS_HEAP_SIZE
#define MACS_HEAP_SIZE 32768
#endif	

class System: public SystemBase
{
public:
	static const uint32_t HEAP_SIZE = MACS_HEAP_SIZE;

	static void InitCpu();
	static void HardFaultHandler();
	static bool SetUpIrqHandling(int irq_num, bool vector, bool enable);
	static void RaiseIrq(int irq_num);

private:
	static void InitClock();
};
