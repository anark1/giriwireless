/** @copyright AstroSoft Ltd */
#pragma once

#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include "common.hpp"

namespace utils
{
 
template <typename T>
class DynArr
{
public:
	typedef T * Iterator;

	static const size_t NPOS = static_cast<size_t>(-1);

	DynArr();
	explicit DynArr(size_t capacity);
	~DynArr();
	void Insert(size_t index, const T & item);
	 
	void AddFront(const T & item)
	{
		Insert(0, item);
	}

	void Add(const T & item)
	{
		Insert(m_count, item);
	}

	bool Remove(const T & item);
	void RemoveAt(size_t index);
	void Clear();
	size_t IndexOf(const T & item);
	bool Contains(const T & item);
	 
	T & operator[](size_t index) const
	{
		_ASSERT(index < m_count);
		return m_items[index];
	}

	T & First() const
	{
		_ASSERT(m_count);
		return m_items[0];
	}
	 
	T & Last() const
	{
		_ASSERT(m_count);
		return m_items[m_count - 1];
	}

	T TakeAt(size_t index);
	 
	T TakeFirst()
	{
		return TakeAt(0);
	}

	T TakeLast()
	{
		return TakeAt(m_count - 1);
	}

	size_t Count() const
	{
		return m_count;
	}

	Iterator Begin() const
	{
		return &m_items[0];
	}

	Iterator End() const
	{
		return &m_items[m_count];
	}

	Iterator Erase(Iterator pos);
	void CopyTo(void * data) const;
	void CopyFrom(const void * data, size_t length);
	 
	template <typename Less>
	void Sort(Less less);

	void Sort();
	void SetCapacity(size_t capacity);

private:
	CLS_COPY(DynArr)

	void EnsureCapacity(size_t capacity);
	void Copy(T * srcArray, size_t srcIndex, T * dstArray, size_t dst_index, size_t length);

	size_t m_count;
	size_t m_capacity;
	T * m_items;
};

template <typename T>
DynArr<T>::DynArr() :
		m_count(0),
		m_capacity(0),
		m_items(nullptr)
{
}

template <typename T>
DynArr<T>::DynArr(size_t capacity) :
		m_count(0),
		m_capacity(capacity),
		m_items(new T[capacity])
{
}

template <typename T>
DynArr<T>::~DynArr()
{
	Clear();
}

template <typename T>
void DynArr<T>::Clear()
{
	if (m_items)
		delete[] m_items;

	m_items = nullptr;
	m_capacity = 0;
	m_count = 0;
}

template <typename T>
void DynArr<T>::Insert(size_t index, const T & item)
{
	_ASSERT(index <= m_count);
	if (index > m_count)
		return;

	if (m_count == m_capacity)
		EnsureCapacity(m_count + 1);

	if (index < m_count)
		Copy(m_items, index, m_items, index + 1, m_count - index);

	m_items[index] = item;
	++m_count;
}

template <typename T>
bool DynArr<T>::Remove(const T & item)
{
	size_t index = IndexOf(item);
	if (index == NPOS)
		return false;

	RemoveAt(index);
	return true;
}

template <typename T>
size_t DynArr<T>::IndexOf(const T & item)
{
	for (size_t i = 0; i < m_count; ++i)
		if (m_items[i] == item)
			return i;

	return NPOS;
}

template <typename T>
inline bool DynArr<T>::Contains(const T & item)
{
	return IndexOf(item) != NPOS;
}

template <typename T>
T DynArr<T>::TakeAt(size_t index)
{
	_ASSERT(index < m_count);

	T item = m_items[index];
	RemoveAt(index);

	return item;
}

template <typename T>
void DynArr<T>::RemoveAt(size_t index)
{
	_ASSERT(index < m_count);
	if (index >= m_count)
		return;

	--m_count;
	if (index < m_count)
		Copy(m_items, index + 1, m_items, index, m_count - index);
}

template <typename T>
typename DynArr<T>::Iterator DynArr<T>::Erase(Iterator pos)
{
	size_t index = pos - Begin();
	RemoveAt(index);

	return &m_items[index];
}

template <typename T>
void DynArr<T>::EnsureCapacity(size_t min_capacity)
{
	static const size_t max_capacity = static_cast<size_t>(-1);
	static const size_t default_capacity = 4;

	if (m_capacity < min_capacity) {
		size_t num = (m_capacity == 0 ? default_capacity : m_capacity * 2);

		if (num > max_capacity)
			num = max_capacity;

		if (num < min_capacity)
			num = min_capacity;

		SetCapacity(num);
	}
}

template <typename T>
void DynArr<T>::SetCapacity(size_t capacity)
{
	T * new_items = new T[capacity];
	if (m_count > 0)
		Copy(m_items, 0, new_items, 0, m_count);

	if (m_items)
		delete[] m_items;

	m_capacity = capacity;
	m_items = new_items;
}

template <typename T>
inline void DynArr<T>::Copy(T * src_array, size_t src_index, T * dst_array, size_t dst_index, size_t length)
{
	memmove(dst_array + dst_index, src_array + src_index, sizeof(T) * length);
}

template <typename T>
template <typename Less>
inline void DynArr<T>::Sort(Less less)
{
	StableSort(Begin(), End(), less);
}

template <typename T>
inline void DynArr<T>::Sort()
{
	Sort(Less<T>());
}

template <typename T>
void DynArr<T>::CopyTo(void * data) const
{
	memcpy(data, m_items, m_count * sizeof(T));
}

template <typename T>
void DynArr<T>::CopyFrom(const void * data, size_t length)
{
	Clear();
	m_count = length / sizeof(T);
	m_capacity = m_count;
	m_items = new T[m_capacity];

	memcpy(m_items, data, length);
}

typedef byte * LIST_PTR;

template <typename T, const size_t next_elm_offset>
class SList
{
private:
	static const size_t m_offset = next_elm_offset;
public:
	static inline const T * & Next(const T * elm)
	{
		return *(const T **)(((LIST_PTR)elm) + m_offset);
	}
	static inline T * & Next(T * elm)
	{
		return *(T **)(((LIST_PTR)elm) + m_offset);
	}
	static ulong Qty(const T * head)
	{
		ulong qty = 0;
		while (head) {
			++qty;
			head = Next(head);
		}
		return qty;
	}
	static T ** Find(T * & head, T * elm)
	{
		T ** ptr = &head;
		while ((*ptr)) {
			if ((*ptr) == elm)
				break;
			ptr = &Next(*ptr);
		}
		return ptr;
	}
	static void Add(T * & head, T * elm)
	{
		_ASSERT(elm);_ASSERT(! Next(elm));  
		_ASSERT(! * Find(head, elm));

		Next(elm) = head;
		head = elm;
	}
	static void Del(T * & head, T * elm)
	{
		_ASSERT(elm);

		T ** ptr = Find(head, elm);
		if ((*ptr)) {
			(*ptr) = Next(elm);
#if MACS_DEBUG
			Next(elm) = nullptr;  
#endif		
		}
	}
	static T * Fetch(T * & head)
	{
		T * elm = head;
		if (head) {
			head = Next(head);
#if MACS_DEBUG
			Next(elm) = nullptr;  
#endif	
		}
		return elm;
	}
	static T * Func(T * & head, bool (*pf)(T *))
	{
		_ASSERT(pf);
		T * elm = head;
		while (elm) {
			if (!(*pf)(elm))
				break;
			elm = Next(elm);
		}
		return elm;
	}
};
#define SLIST_NEXT_OFFSET(type, next)		((size_t) & ((type *) 0)->next)
#ifndef MACS_CCC
#define SLIST_DECLARE(name, type, next) \
		static const size_t name##next_elm_offset = SLIST_NEXT_OFFSET(type, next); \
		typedef SList<type, name##next_elm_offset> name
#else	 
#define SLIST_DECLARE(name, type, next) typedef SList<type, 0> name
#endif

template <typename T, size_t next_elm_offset, bool Preceeding(T * a, T * b)>  
class SListOrd: public SList<T, next_elm_offset>
{
public:
	static void Add(T * & head, T * elm)
	{
		T ** ptr = &head;
		while ((*ptr)) {
			_ASSERT((* ptr) != elm);
			if (Preceeding(elm, (*ptr)))
				break;
			ptr = &SList<T, next_elm_offset>::Next(*ptr);
		}
		SList<T, next_elm_offset>::Add((*ptr), elm);
	}
};
#ifndef MACS_CCC 
#define SLISTORD_DECLARE(name, type, next, less) \
		static const size_t name##next_elm_offset = SLIST_NEXT_OFFSET(type, next); \
		typedef SListOrd<type, name##next_elm_offset, less> name
#else	 
#define SLISTORD_DECLARE(name, type, next, less) typedef SListOrd<type, 0, less> name
#endif		

}  

using namespace utils;
