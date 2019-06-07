/** @copyright AstroSoft Ltd */

#include "memory_manager.hpp"
#include <new>

#if defined(__ICCARM__) || defined(__VISUALDSPVERSION__)		 

void * operator new(size_t size)
{
	return MemoryManager::Allocate(size);
}

void operator delete(void * ptr)
{
	MemoryManager::Deallocate(ptr);
}

void * operator new[](size_t size)
{
	return MemoryManager::Allocate(size);
}

void operator delete[](void * ptr)
{
	MemoryManager::Deallocate(ptr);
}

#else		 

void * operator new(size_t size, const std::nothrow_t &) throw()
{
	return MemoryManager::Allocate(size);
}

void operator delete(void * ptr, const std::nothrow_t &) throw()
{
	MemoryManager::Deallocate(ptr);
}

void * operator new(size_t size) throw(std::bad_alloc)
{
	return MemoryManager::Allocate(size);
}

void operator delete(void * ptr) throw()
{
	MemoryManager::Deallocate(ptr);
}

void * operator new[](size_t size, const std::nothrow_t &) throw()
{
	return MemoryManager::Allocate(size);
}

void operator delete[](void * ptr, const std::nothrow_t &) throw()
{
	MemoryManager::Deallocate(ptr);
}

void * operator new[](size_t size) throw(std::bad_alloc)
{
	return MemoryManager::Allocate(size);
}

void operator delete[](void * ptr) throw()
{
	MemoryManager::Deallocate(ptr);
}

#endif		 
