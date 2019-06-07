/** @copyright AstroSoft Ltd */
#pragma once

#include <stdint.h>

namespace macs
{

struct SWStackFrameRegisters
{
	uint32_t R4;
	uint32_t R5;
	uint32_t R6;
	uint32_t R7;
	uint32_t R8;
	uint32_t R9;
	uint32_t R10;
	uint32_t R11;
};

 
struct SoftwareStackFrame
{
	uint32_t EXC_RETURN;
	::SWStackFrameRegisters SWStackFrameRegisters;
};

struct SWStackFrameRegistersFPU
{
	uint32_t S16;
	uint32_t S17;
	uint32_t S18;
	uint32_t S19;
	uint32_t S20;
	uint32_t S21;
	uint32_t S22;
	uint32_t S23;
	uint32_t S24;
	uint32_t S25;
	uint32_t S26;
	uint32_t S27;
	uint32_t S28;
	uint32_t S29;
	uint32_t S30;
	uint32_t S31;
};

struct SWStackFrameFPU: SoftwareStackFrame
{
	::SWStackFrameRegistersFPU SWStackFrameRegistersFPU;
};

struct HardwareStackFrame
{
	uint32_t R0;
	uint32_t R1;
	uint32_t R2;
	uint32_t R3;
	uint32_t R12;
	uint32_t LR;
	uint32_t PC;
	uint32_t XPSR;
};

struct HWStackFrameFPU: HardwareStackFrame
{
	uint32_t S0;
	uint32_t S1;
	uint32_t S2;
	uint32_t S3;
	uint32_t S4;
	uint32_t S5;
	uint32_t S6;
	uint32_t S7;
	uint32_t S8;
	uint32_t S9;
	uint32_t S10;
	uint32_t S11;
	uint32_t S12;
	uint32_t S13;
	uint32_t S14;
	uint32_t S15;
	uint32_t FPSCR;
	uint32_t dummy;
};

struct StackFrame
{
	::SoftwareStackFrame SoftwareStackFrame;
	::HardwareStackFrame HardwareStackFrame;
};

struct StackFrameFPU
{
	::SWStackFrameFPU SWStackFrameFPU;
	::HWStackFrameFPU HWStackFrameFPU;
};

class StackFramePtr
{
public:
	 
	static HardwareStackFrame * getHWFramePtr(void * ptr)
	{
		return &(static_cast<StackFrame*>(ptr))->HardwareStackFrame;
	}

	static uint32_t getStackFrameSize()
	{
		return sizeof(StackFrame);
	}

	static uint32_t getInitial_EXC_RETURN()
	{
		return 0xFFFFFFFD;
	}
};

}  
