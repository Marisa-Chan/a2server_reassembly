#ifndef BIT_SET_H
#define BIT_SET_H

#include <inttypes.h>

template<int BIT_SET_SIZE>
struct BitSet
{
	uint32_t _storage[(BIT_SET_SIZE + 31) / 32];

	void Set(uint32_t index)
	{
		//509cfb
		if (index < BIT_SET_SIZE)
			_storage[index >> 5] |= 1 << (index & 0x1F);
	}

	void Unset(uint32_t index)
	{
		//509d4c
		if (index < BIT_SET_SIZE)
			_storage[index >> 5] &= ~(1 << (index & 0x1F));
	}

	uint32_t AllocBit()  // Set 0 bit to 1 and return index
	{
		//509d9f
		uint32_t i = 0;
		uint32_t j = 0;
		for (; _storage[i] == 0xFFFFFFFF; i++)
			{}
		for (; _storage[i] & (1 << j); j++)
			{}

		uint32_t index = (i << 5) + j;
		Set(index);
		return index;
	}

	void Clear()
	{
		//509cd8
		memset(_storage, 0, sizeof(_storage));
		_storage[0] = 1; // Set 0 element to used
	}
};


#endif 