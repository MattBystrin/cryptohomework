#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <stdexcept>
#include <functional>

template <typename T>
class bit_adapter 
{
public:
	bit_adapter(std::vector<T> &seq): data(seq) {}
	void set_at(int idx, int val)
	{
		int num = idx / dsize;
		int bit = idx % dsize;
		if (num >= data.size())
			throw std::runtime_error("Out of range");
		if (val)
			data[num] |= 0x01 << bit;
		else
			data[num] &= ~(0x01 << bit);
	}
	uint8_t operator[](int idx)
	{
		int num = idx / dsize;
		int bit = idx % dsize;
		if (num >= data.size())
			throw std::runtime_error("Out of range");
		return (data[num] >> bit) & 0x01;
	}
	int size() { return dsize * data.size(); }
private:
	const int dsize = sizeof(T) * 8;
	std::vector<T> &data;
};

#endif
