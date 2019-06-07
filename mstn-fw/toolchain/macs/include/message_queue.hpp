/** @copyright AstroSoft Ltd */
#pragma once

#include <stddef.h>
#include "common.hpp"
#include "critical_section.hpp"
#include "semaphore.hpp"

namespace macs
{

template <typename T>
class MessageQueueInterface
{
public:
	virtual Result Push(const T & message, uint32_t timeout_ms = INFINITE_TIMEOUT) = 0;
	virtual Result PushFront(const T & message, uint32_t timeout_ms = INFINITE_TIMEOUT) = 0;
	virtual Result Pop(T & message, uint32_t timeout_ms = INFINITE_TIMEOUT) = 0;
	virtual Result Peek(T & message, uint32_t timeout_ms = INFINITE_TIMEOUT) = 0;
	virtual size_t Count() const = 0;
	virtual size_t GetMaxSize() const = 0;
};
 
template <typename T>
class MessageQueue: public MessageQueueInterface<T>
{
public:
	MessageQueue(size_t max_size, T * mem = nullptr);
	~MessageQueue();
	virtual Result Push(const T & message, uint32_t timeout_ms = INFINITE_TIMEOUT);
	virtual Result PushFront(const T & message, uint32_t timeout_ms = INFINITE_TIMEOUT);
	virtual Result Pop(T & message, uint32_t timeout_ms = INFINITE_TIMEOUT);
	virtual Result Peek(T & message, uint32_t timeout_ms = INFINITE_TIMEOUT);
	virtual size_t Count() const;
	virtual size_t GetMaxSize() const
	{
		return m_len - 1;
	}

private:
	CLS_COPY(MessageQueue)

	enum ACTION
	{
		QA_PUSH_FRONT,
		QA_PUSH_BACK,
		QA_POP,
		QA_PEEK
	};
	Result ProcessMessage(Semaphore & wait_sem, Semaphore & sig_sem, T & message, ACTION action, uint32_t timeout_ms);

private:
	const size_t m_len;
	Semaphore m_sem_read;
	Semaphore m_sem_write;

	bool m_is_alien_mem;
	T * m_memory;
	T * m_head_ptr, *m_tail_ptr;  
};

template <typename T>
MessageQueue<T>::MessageQueue(size_t max_size, T * mem) :
		m_len(max_size + 1),  
		m_sem_read(0, max_size),
		m_sem_write(max_size, max_size)
{
	if (!mem) {
		m_memory = new T[m_len];
		m_is_alien_mem = false;
	} else {
		m_memory = mem;
		m_is_alien_mem = true;
	}
	m_head_ptr = m_tail_ptr = m_memory;
}

template <typename T>
MessageQueue<T>::~MessageQueue()
{
	if (!m_is_alien_mem)
		delete[] m_memory;
}

template <typename T>
inline size_t MessageQueue<T>::Count() const
{
	long diff = m_tail_ptr - m_head_ptr;
	return diff >= 0 ? diff : (m_len + diff);
}

template <typename T>
inline Result MessageQueue<T>::Push(const T & message, uint32_t timeout_ms)
{
	return ProcessMessage(m_sem_write, m_sem_read, const_cast<T &>(message), QA_PUSH_BACK, timeout_ms);
}

template <typename T>
inline Result MessageQueue<T>::PushFront(const T & message, uint32_t timeout_ms)
{
	return ProcessMessage(m_sem_write, m_sem_read, const_cast<T &>(message), QA_PUSH_FRONT, timeout_ms);
}

template <typename T>
inline Result MessageQueue<T>::Pop(T & message, uint32_t timeout_ms)
{
	return ProcessMessage(m_sem_read, m_sem_write, message, QA_POP, timeout_ms);
}

template <typename T>
inline Result MessageQueue<T>::Peek(T & message, uint32_t timeout_ms)
{
	return ProcessMessage(m_sem_read, m_sem_read, message, QA_PEEK, timeout_ms);
}

template <typename T>
Result MessageQueue<T>::ProcessMessage(Semaphore & wait_sem, Semaphore & sig_sem, T & message, ACTION action, uint32_t timeout_ms)
{
	if (!Sch().IsInitialized() || !Sch().IsStarted())
		return ResultErrorInvalidState;

	if (!timeout_ms) {
		if (!System::IsSysCallAllowed())
			return ResultErrorSysCallNotAllowed;
	} else {
		if (System::IsInInterrupt())
			return ResultErrorInterruptNotSupported;
	}

	Result retcode = wait_sem.Wait(System::IsInInterrupt() ? 0 : timeout_ms);

	if (retcode != ResultOk)
		return retcode;

	{
		PauseSection _ps_;
		switch (action) {
		case QA_PUSH_FRONT:
		{
			_ASSERT(Count() < GetMaxSize());
			if (m_head_ptr == m_memory)
				m_head_ptr = &m_memory[m_len];
			*--m_head_ptr = message;
		}
			break;
		case QA_PUSH_BACK:
		{
			_ASSERT(Count() < GetMaxSize());
			*m_tail_ptr++ = message;
			if (m_tail_ptr - m_memory == m_len)
				m_tail_ptr = m_memory;
		}
			break;
		case QA_POP:
		{
			_ASSERT(Count() != 0);
			message = *m_head_ptr++;
			if (m_head_ptr - m_memory == m_len)
				m_head_ptr = m_memory;
		}
			break;
		case QA_PEEK:
		{
			_ASSERT(Count() != 0);
			message = *m_head_ptr;
		}
			break;
		}
	}

	retcode = sig_sem.Signal();

	return retcode;
}

}  
