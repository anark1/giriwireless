/** @copyright AstroSoft Ltd */

#pragma once

#include "common.hpp"

namespace macs
{

typedef uint8_t byte;
 
class Buf
{
public:
	static const size_t DEF_BUF_SIZE = 64;

protected:
	enum BUF_STATE
	{
		BS_OWN = (0x01 << 0),
		BS_DYN = (0x01 << 1),
		BS_DYN_OWN = BS_DYN | BS_OWN
	};

	BitMask<BUF_STATE> m_state;
	size_t m_len;
	uint m_beg;
	byte *m_mem;
	byte *m_mem_aligned;
	size_t m_size;

public:
	 
	Buf()
	{
		Reset();
	}

	~Buf()
	{
		FreeMem();
	}

	inline size_t Len() const
	{
		return m_len;
	}

	inline size_t Size() const
	{
		return m_size;
	}

	inline size_t Rest() const
	{
		return Size() - Len();
	}

	inline byte *Data()
	{
		return &m_mem_aligned[m_beg];
	}

	inline const byte *Data() const
	{
		return &((const byte *)m_mem_aligned)[m_beg];
	}

	inline byte operator[](size_t i) const
	{
		_ASSERT(i < Len());
		return Data()[i];
	}

	bool operator ==(const Buf &buf) const;

	inline void Reset()
	{
		m_beg = m_len = 0;
	}

	bool Alloc(size_t size, size_t alignment = 1);

	void Free();
	 
	inline void AddLen(size_t len)
	{
		_ASSERT(Len() + len <= Size());
		m_len += len;
	}

	byte ReadByte()
	{
		_ASSERT(m_len >= 1);
		m_len -= 1;
		m_beg += 1;
		return *(Data() - 1);
	}

	inline int16_t ReadInt16()
	{
		int16_t val;
		Read((byte *)&val, sizeof(val));
		return val;
	}

	inline int32_t ReadInt32()
	{
		int32_t val;
		Read((byte *)&val, sizeof(val));
		return val;
	}

	void Read(byte *ptr, size_t len, bool move_pos = true);
	inline void Read(byte *ptr, size_t len) const
	{
		((Buf *)this)->Read(ptr, len, false);
	}
	inline void Read(byte *ptr, size_t len, bool move_pos) const
	{
		_ASSERT(move_pos == false);
		Read(ptr, len);
	}

	inline void AddByte(byte val)
	{
		*(Data() + m_len) = val;
		AddLen(1);
	}

	inline void AddInt16(int16_t val)
	{
		Add((const byte *)&val, sizeof(val));
	}

	inline void AddInt32(int32_t val)
	{
		Add((const byte *)&val, sizeof(val));
	}

	void Add(const byte *ptr, size_t len);

	void Add(const Buf & buf)
	{
		Add(buf.Data(), buf.Len());
	}

	void Copy(const byte *ptr, size_t len);

	inline void Copy(const Buf &buf)
	{
		Copy(buf.Data(), buf.Len());
	}

	void Grab(byte *ptr, size_t len, bool dupe = false);
	 
	inline void Dupe(const byte *ptr, size_t len)
	{
		Grab((byte *)ptr, len, true);
	}
	 
	void Grab(Buf &buf, bool dupe = false);
	 
	inline void Dupe(Buf &buf)
	{
		Grab(buf, true);
	}
	 
	void Trans(Buf &src, size_t len);

private:
	CLS_COPY(Buf)

	inline void FreeMem()
	{
		if (m_mem && m_state.Check(BS_DYN_OWN))
			delete[] m_mem;
	}
};

template <size_t buf_size = Buf::DEF_BUF_SIZE>
class StatBuf: public Buf
{
private:
	byte m_buf[buf_size];

public:
	StatBuf()
	{
		m_mem = m_mem_aligned = m_buf;
		m_size = buf_size;
	}
};

typedef StatBuf<Buf::DEF_BUF_SIZE> DefStatBuf;
 
class DynBuf: public Buf
{
public:
	 
	DynBuf(size_t size = 0)
	{
		m_state.Add(BS_DYN_OWN);
		m_size = 0;
		m_mem = m_mem_aligned = nullptr;
		if (size)
			Alloc(size);
	}
};

}  
