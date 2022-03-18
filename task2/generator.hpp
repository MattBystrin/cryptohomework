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
	bit_adapter(const std::vector<T> &seq): data(seq) {}
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
	const std::vector<T> &data;
};

template <typename T>
class generator {
	using data_type = T;
public:
	generator(std::vector<int> _pws): pws(_pws)
	{
		int bitlen = *std::max_element(pws.begin(), pws.end());
		outbit = dsize - bitlen % dsize - 1;
		reg.resize(bitlen % dsize == 0 ? bitlen / dsize
					       : bitlen / dsize + 1);
		for (auto i = reg.begin(); i != reg.end(); ++i) {
			*i = std::rand();
		}
	}
	template <typename Td>
	Td generate()
	{
		Td seq = 0;
		for(int i = 0; i < sizeof(Td) * 8 - 1; i++) {
			seq |= shift();
			seq <<= 1;
		}
		seq |= shift();
		return seq;
	}
	/* 
	 * Shifts reg and return bit 
	 */
	uint8_t shift() 
	{
		uint8_t fb = feedback();
		auto i = reg.rbegin();
		uint8_t output = 0x01 & (*i >> outbit);
		*i <<= 1;
		++i;
		for (; i != reg.rend(); ++i) {
			*(i - 1) |= *i >> (dsize - 1);
			*i <<= 1;
		}
		*reg.begin() |= fb;
		return output;
	}
	void load(std::vector<data_type> r)
	{
		std::fill(reg.begin(), reg.end(), 0);
		for (int i = 0; i < reg.size(); ++i) {
			reg[i] = r[i];
		}
	}
	~generator(){};
private:
	int outbit;
	std::vector<data_type> reg; /* Contains register data */
	std::vector<int> pws; /* Contains polynoms powers */
	constexpr static int dsize = sizeof(data_type) * 8;
	uint8_t feedback()
	{
		uint8_t output = 0;
		for (auto i = pws.begin(); i != pws.end(); ++i) {
			if (*i == 0) 
				output ^= 1;
			int num = (*i - 1) / dsize;
			int bit = (*i - 1) % dsize; 
			if (num > reg.size())
				throw std::runtime_error("Out of range");
			output ^= (reg[num] >> bit) & 0x01;
		}
		return output;
	}
};

#endif
