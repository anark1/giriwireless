/** @copyright AstroSoft Ltd */

#include <stdlib.h>
#include <stdio.h>
#include "memory_manager.hpp"
#include "scheduler.hpp"
#include "task.hpp"

#if MACS_USE_MPU
void MPU_Init()
{
	System::MpuInit();

#if MACS_MPU_PROTECT_NULL
	MPU_SetMine(MPU_ZERO_ADR_MINE, 0x00000000U);
#endif

#if MACS_MPU_PROTECT_STACK
	MPU_SetMine(MPU_MAIN_STACK_MINE, ((uint32_t)(((byte *)(System::MacsMainStackBottom - System::MacsMainStackSize)) + (0x20 - 1))) & ~0x1F);
#endif
}

void MPU_SetMine(MPU_MINE_NUM rnum, uint32_t adr)
{
	_ASSERT((adr & 0x1F) == 0);
	System::MpuSetMine(rnum, adr);
}

void MPU_RemoveMine(MPU_MINE_NUM rnum)
{
	System::MpuRemoveMine(rnum);
}
#endif

bool MemoryManager::m_init_flag = false;
size_t MemoryManager::m_heap_size = 0;
byte MemoryManager::m_lock = 0;

#if MACS_MEM_STATISTICS
size_t MemoryManager::m_cur_heap_size = 0;
size_t MemoryManager::m_peak_heap_size = 0;
#if MACS_DEBUG
static volatile long dbgCurHeapSize, dbgCurHeapChange;  
#endif
#endif

void MemoryManager::LogAllocatedSize()
{
#if MACS_MEM_STATISTICS && MACS_PRINTF_ALLOWED
	printf("memory allocated: %d\n\r", (int)m_cur_heap_size);
#endif
}

void * MemoryManager::MemAlloc(size_t size)
{
#if MACS_MEM_STATISTICS
	if (m_cur_heap_size + size > m_heap_size)
		return nullptr;
	size_t * ph = (size_t *)malloc(size + sizeof(size_t));
	if (!ph)
		return nullptr;
	*ph = size;
	m_cur_heap_size += size;
#if MACS_DEBUG
	dbgCurHeapChange = size;
	dbgCurHeapSize = m_cur_heap_size;
#endif
	if (m_cur_heap_size > m_peak_heap_size)
		m_peak_heap_size = m_cur_heap_size;
	return ph + 1;
#else
	return malloc(size);
#endif
}

void MemoryManager::MemFree(void * ptr)
{
#if MACS_MEM_STATISTICS
	size_t * ph = (size_t *)ptr - 1;
	m_cur_heap_size -= *ph;
#if MACS_DEBUG
	dbgCurHeapChange = -(long)*ph;
	dbgCurHeapSize = m_cur_heap_size;
#endif
#if MACS_MEM_WIPE
	Wipe(ptr, *ph);
#endif
	free(ph);
#else
	free(ptr);
#endif
}

void* MemoryManager::Allocate(size_t size)
{
	if (!m_init_flag)
		Initialize<HEAP_SIZE>();

	if (size == 0)
		return nullptr;

	void * ptr;
	for (;;) {
		{
			PauseSection _ps_;
			{
				HeapLocker _hl_;
				ptr = MemAlloc(size);
			}
		}
		if (ptr)
			break;

		ALARM_ACTION act = App().OnAlarm(AR_OUT_OF_MEMORY);
		if (act == AA_CONTINUE)
			continue;

		MACS_CRASH(AR_OUT_OF_MEMORY);
	}_ASSERT(ptr != nullptr);

	return ptr;
}

void MemoryManager::Deallocate(void* ptr)
{
	if (!m_init_flag)
		Initialize<HEAP_SIZE>();

	if (!ptr)
		return;

	{
		PauseSection _ps_;
		{
			HeapLocker _hl_;
			MemFree(ptr);
		}
	}
}
