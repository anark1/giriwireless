/** @copyright AstroSoft Ltd */
#pragma once

#include "system_cfg.h"
#include "config.hpp"

#ifndef MACS_DEBUG
#define MACS_DEBUG               0        
#endif

#ifndef MACS_INIT_TICK_RATE_HZ
#define MACS_INIT_TICK_RATE_HZ   1000u    
#endif

#ifndef MACS_MAX_STACK_SIZE
#define MACS_MAX_STACK_SIZE      0x800u   
#endif

#ifndef MACS_MAIN_STACK_SIZE
#define MACS_MAIN_STACK_SIZE     1000u    
#endif

#ifndef MACS_WATCH_STACK
#define MACS_WATCH_STACK         0	      
#endif

#ifndef MACS_TASK_NAME_LENGTH
#define MACS_TASK_NAME_LENGTH   -1        
#endif

#ifndef MACS_MAX_TASK_PRIORITY
#define MACS_MAX_TASK_PRIORITY	PriorityRealtime   
#endif 

#ifndef MACS_MUTEX_PRIORITY_INVERSION		 
#define MACS_MUTEX_PRIORITY_INVERSION 1   
#endif

#ifndef MACS_PROFILING_ENABLED
#define MACS_PROFILING_ENABLED   0      
#endif

#ifndef MACS_MEM_STATISTICS
#define MACS_MEM_STATISTICS      0      
#endif

#ifndef MACS_MEM_WIPE
#define MACS_MEM_WIPE            0      
#endif

#ifndef MACS_IRQ_FAST_SWITCH
#define MACS_IRQ_FAST_SWITCH     1      
#endif

#ifndef MACS_SLEEP_ON_IDLE
#define MACS_SLEEP_ON_IDLE       0      
#endif

#ifndef MACS_PRINTF_ALLOWED
#define MACS_PRINTF_ALLOWED      0      
#endif
