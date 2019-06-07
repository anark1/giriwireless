/** @copyright AstroSoft Ltd */
#pragma once

#ifndef UINT32_MAX
#define UINT32_MAX 0xFFFFFFFFu
#endif	

#ifndef ABS
#define ABS(v) ((v) < 0 ? -(v) : (v))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
 
#define countof(arr) (sizeof(arr) / sizeof((arr)[0])) 

namespace utils
{
typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef const char * CSPTR;
typedef char * SPTR;

#ifdef __ECC__
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#endif	

#define BYTE_MAX UCHAR_MAX	

template <typename T>
T RemapValue(T x, T in_min, T in_max, T out_min, T out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

 
template <typename T1, typename T2>
struct Pair
{
public:
	 
	Pair() :
			first(0),
			second(0)
	{
	}
	 
	Pair(T1 first, T2 second) :
			first(first),
			second(second)
	{
	}
	 
	bool operator<(const Pair<T1, T2>& b) const
	{
		return this->first < b.first;
	}

	T1 first;
	T2 second;
};

template <typename T>
struct Less
{
	bool operator()(const T& a, const T& b) const
	{
		return a < b;
	}
};

template <typename T>
inline void Swap(T &value1, T &value2)
{
	const T t = value1;
	value1 = value2;
	value2 = t;
}
 
template <typename Iterator, typename T, typename Less>
Iterator LowerBound(Iterator begin, Iterator end, const T &value, Less less)
{
	Iterator middle;
	int n = int(end - begin);
	int half;

	while (n > 0) {
		half = n >> 1;
		middle = begin + half;
		if (less(*middle, value)) {
			begin = middle + 1;
			n -= half + 1;
		} else {
			n = half;
		}
	}
	return begin;
}

template <typename Iterator, typename T, typename Less>
Iterator UpperBound(Iterator begin, Iterator end, const T &value, Less less)
{
	Iterator middle;
	int n = end - begin;
	int half;

	while (n > 0) {
		half = n >> 1;
		middle = begin + half;
		if (less(value, *middle)) {
			n = half;
		} else {
			begin = middle + 1;
			n -= half + 1;
		}
	}
	return begin;
}

template <typename Iterator>
void Reverse(Iterator begin, Iterator end)
{
	--end;
	while (begin < end) {
		Swap(*begin++, *end--);
	}
}

template <typename Iterator>
void Rotate(Iterator begin, Iterator middle, Iterator end)
{
	Reverse(begin, middle);
	Reverse(middle, end);
	Reverse(begin, end);
}

template <typename Iterator, typename Less>
void Merge(Iterator begin, Iterator pivot, Iterator end, Less less)
{
	const int len1 = pivot - begin;
	const int len2 = end - pivot;

	if (len1 == 0 || len2 == 0) {
		return;
	}

	if (len1 + len2 == 2) {
		if (less(*(begin + 1), *(begin))) {
			Swap(*begin, *(begin + 1));
		}
		return;
	}

	Iterator firstCut;
	Iterator secondCut;
	int len2Half;
	if (len1 > len2) {
		const int len1Half = len1 / 2;
		firstCut = begin + len1Half;
		secondCut = LowerBound(pivot, end, *firstCut, less);
		len2Half = secondCut - pivot;
	} else {
		len2Half = len2 / 2;
		secondCut = pivot + len2Half;
		firstCut = UpperBound(begin, pivot, *secondCut, less);
	}

	Rotate(firstCut, pivot, secondCut);
	const Iterator newPivot = firstCut + len2Half;
	Merge(begin, firstCut, newPivot, less);
	Merge(newPivot, secondCut, end, less);
}

template <typename Iterator, typename Less>
void StableSort(Iterator begin, Iterator end, Less less)
{
	if (begin == end) {
		return;
	}

	const int span = end - begin;
	if (span < 2) {
		return;
	}

	const Iterator middle = begin + span / 2;
	StableSort(begin, middle, less);
	StableSort(middle, end, less);
	Merge(begin, middle, end, less);
}
}  

using namespace utils;
