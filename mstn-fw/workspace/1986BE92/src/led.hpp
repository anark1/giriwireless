#pragma once
/*






openocd -f interface/jlink.cfg -f target/mdr32f9q2i.cfg -c "init" -c "reset init" -c 
"flash write_image erase /home/alkuj/Загрузки/mm/workspace/1986BE92/1986BE92_blink.elf" -c "reset" -c "shutdown"













*/
#include <stdint.h>
#include "tunes.h"
#include "MDR32F9Qx_port.h"
#include "MDR32F9Qx_rst_clk.h"

#define LED0  (1<<8)
#define LED1  (1<<9)
//#define LED1  (1<<10)

const uint16_t led_tbl[] = {
	LED0,
	LED1};

#define NUM_LED  2

class LedDriver
{
public:

	LedDriver()
	{
		RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTB, ENABLE);

		PORT_InitTypeDef LedPort_structInit;
		LedPort_structInit.PORT_Pin = LED0 | LED1;
		LedPort_structInit.PORT_FUNC = PORT_FUNC_PORT;
		LedPort_structInit.PORT_GFEN = PORT_GFEN_OFF;
		LedPort_structInit.PORT_MODE = PORT_MODE_DIGITAL;
		LedPort_structInit.PORT_OE = PORT_OE_OUT;
		LedPort_structInit.PORT_PD = PORT_PD_DRIVER;
		LedPort_structInit.PORT_PD_SHM = PORT_PD_SHM_OFF;
		LedPort_structInit.PORT_PULL_DOWN = PORT_PULL_DOWN_OFF;
		LedPort_structInit.PORT_PULL_UP = PORT_PULL_UP_OFF;
		LedPort_structInit.PORT_SPEED = PORT_SPEED_MAXFAST;
		PORT_Init(MDR_PORTB, &LedPort_structInit);
	}

	void Toggle(uint16_t i)
	{
		if (i >= NUM_LED)
			return;
		PORT_WriteBit(MDR_PORTB, led_tbl[i], (BitAction)!PORT_ReadInputDataBit(MDR_PORTB, led_tbl[i]));
	}

	uint16_t GetNum() const
	{
		return NUM_LED;
	}

};
