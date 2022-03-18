#ifndef MD4_HPP
#define MD4_HPP

#include <stdint.h>
#include <algorithm>
#include <vector>
#include <array>

class md4 {
public:
	md4(std::array<uint32_t,4> start = {
		0x67452301,
		0xEFCDAB89,
		0x98BADCFE,
		0x10325476
	}, int rounds = 3);
	//void update(uint8_t *in, size_t len) {};
	void update(std::vector<uint8_t> in);
	std::array<uint8_t,16> finish();

private:
	static const size_t max_len = 64;
	uint8_t buf[max_len];
	size_t buf_len = 0;
	uint64_t in_len = 0;
	int rounds = 3;

	std::array<uint32_t,4> output = {
		0x67452301, /* A */
	 	0xEFCDAB89, /* B */
		0x98BADCFE, /* C */
	 	0x10325476  /* D */
	};

	void transform();

	inline uint32_t F(uint32_t x, uint32_t y, uint32_t z)
	{
		return x & y | ~x & z;
	}

	inline uint32_t G(uint32_t x, uint32_t y, uint32_t z)
	{
		return x & y |  x & z | y & z;
	}

	inline uint32_t H(uint32_t x, uint32_t y, uint32_t z)
	{
		return x ^ y ^ z;
	}

	inline uint32_t rotate_left(uint32_t x, int n)
	{
		return x << n | x >> (32 - n);
	}

	inline void FF(uint32_t &a, uint32_t b, uint32_t c, uint32_t d,
			   uint32_t x, uint32_t s)
	{
		a = rotate_left(a + F(b, c, d) + x,  s);
	}

	inline void GG(uint32_t &a, uint32_t b, uint32_t c, uint32_t d,
			   uint32_t x, uint32_t s)
	{
		a = rotate_left(a + G(b, c, d) + x + 0x5A827999,  s);
	}

	inline void HH(uint32_t &a, uint32_t b, uint32_t c, uint32_t d,
			   uint32_t x, uint32_t s)
	{
		a = rotate_left(a + H(b, c, d) + x + 0x6ED9EBA1,  s);
	}
};

#endif
