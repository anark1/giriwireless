/** @copyright AstroSoft Ltd */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include "system.hpp"
#include "application.hpp"
#include "scheduler.hpp"

class MemoryManager
{
private:
	static const uint32_t HEAP_SIZE = System::HEAP_SIZE;

	static byte m_lock;
#if MACS_MEM_STATISTICS
	static size_t m_cur_heap_size, m_peak_heap_size;
#endif

	class HeapLocker
	{
	public:
		HeapLocker()
		{
			PauseSection _ps_;
			if (m_lock)
				App().OnAlarm(AR_MEM_LOCKED);
			m_lock = 1;
		}

		~HeapLocker()
		{
			PauseSection _ps_;
			m_lock = 0;
		}
	};

public:
	template <uint32_t heapSize>
	static void Initialize();

	static void * Allocate(size_t size);
	static void Deallocate(void * ptr);
#if MACS_MEM_WIPE
	static void Wipe(void * ptr, size_t size)
	{
		memset(ptr, 0xCC, size);
	}
#endif	

#if MACS_MEM_STATISTICS
	static size_t MaxHeapSize()
	{
		return m_heap_size;
	}
	static size_t CurHeapSize()
	{
		return m_cur_heap_size;
	}
	static size_t PeakHeapSize()
	{
		return m_peak_heap_size;
	}
#endif

private:
	MemoryManager();
	~MemoryManager();

	CLS_COPY(MemoryManager)

	static void * MemAlloc(size_t size);
	static void MemFree(void * ptr);

	static void LogAllocatedSize();

	static bool m_init_flag;
	static size_t m_heap_size;
};

template <uint32_t heapSize>
void MemoryManager::Initialize()
{
	m_heap_size = heapSize;
	m_init_flag = true;
}
