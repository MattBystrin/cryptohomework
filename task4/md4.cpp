#include "md4.hpp"

md4::md4(std::array<uint32_t, 4> start, int rounds)
{
	std::fill(buf, buf + max_len, 0);
	output = start;
	this->rounds = rounds;
}

void md4::update(std::vector<uint8_t> in)
{
	if (in.empty())
		return;
	in_len += in.size() * 8;
	auto i = in.begin();
	while (std::distance(i, in.end()) >= max_len - buf_len) {
		std::copy(i, i + max_len - buf_len, buf + buf_len);
		i += max_len - buf_len;
		transform();
		buf_len = 0;
	}
	std::copy(i, in.end(), buf + buf_len);
	buf_len += std::distance(i, in.end());
}

std::array<uint8_t,16> md4::finish()
{
	buf[buf_len++] = 0x80;
	if (buf_len < 57) {
		std::fill(buf + buf_len, buf + 56, 0);
	} else {
		std::fill(buf + buf_len, buf + max_len, 0);
		transform();
		buf_len = 0;
		std::fill(buf, buf + 56, 0);
	}
	std::copy((uint8_t *)&in_len, (uint8_t *)&in_len + 8, buf + 56);
	transform();
	buf_len = 0;
	in_len = 0;
	std::fill(buf, buf + max_len, 0);
	std::array<uint8_t,16> o;
	std::copy((uint8_t *)&output[0], (uint8_t *)&output[0] + 16, &o[0]);
	return o;
}


void md4::transform()
{
	uint32_t x[16];
	uint32_t a = output[0];
	uint32_t b = output[1];
	uint32_t c = output[2];
	uint32_t d = output[3];

	std::copy((uint32_t *)buf, (uint32_t *)buf + max_len/4, x);
	/* First round */
	if (rounds < 1)
		return;
	FF(a, b, c, d, x[0], 3);	
	FF(d, a, b, c, x[1], 7);	
	FF(c, d, a, b, x[2], 11);	
	FF(b, c, d, a, x[3], 19);	
	FF(a, b, c, d, x[4], 3);	
	FF(d, a, b, c, x[5], 7);	
	FF(c, d, a, b, x[6], 11);	
	FF(b, c, d, a, x[7], 19);	
	FF(a, b, c, d, x[8], 3);	
	FF(d, a, b, c, x[9], 7);	
	FF(c, d, a, b, x[10], 11);	
	FF(b, c, d, a, x[11], 19);	
	FF(a, b, c, d, x[12], 3);	
	FF(d, a, b, c, x[13], 7);	
	FF(c, d, a, b, x[14], 11);	
	FF(b, c, d, a, x[15], 19);	
	/* Second round */
	if (rounds > 1) {
		GG(a, b, c, d, x[0], 3);	
		GG(d, a, b, c, x[4], 5);	
		GG(c, d, a, b, x[8], 9);	
		GG(b, c, d, a, x[12], 13);	
		GG(a, b, c, d, x[1], 3);	
		GG(d, a, b, c, x[5], 5);	
		GG(c, d, a, b, x[9], 9);	
		GG(b, c, d, a, x[13], 13);	
		GG(a, b, c, d, x[2], 3);	
		GG(d, a, b, c, x[6], 5);	
		GG(c, d, a, b, x[10], 9);	
		GG(b, c, d, a, x[14], 13);	
		GG(a, b, c, d, x[3], 3);	
		GG(d, a, b, c, x[7], 5);	
		GG(c, d, a, b, x[11], 9);	
		GG(b, c, d, a, x[15], 13);	
		/* Third round */	
		if (rounds > 2) {
			HH(a, b, c, d, x[0], 3);	
			HH(d, a, b, c, x[8], 9);	
			HH(c, d, a, b, x[4], 11);	
			HH(b, c, d, a, x[12], 15);	
			HH(a, b, c, d, x[2], 3);	
			HH(d, a, b, c, x[10], 9);	
			HH(c, d, a, b, x[6], 11);	
			HH(b, c, d, a, x[14], 15);	
			HH(a, b, c, d, x[1], 3);	
			HH(d, a, b, c, x[9], 9);	
			HH(c, d, a, b, x[5], 11);	
			HH(b, c, d, a, x[13], 15);	
			HH(a, b, c, d, x[3], 3);	
			HH(d, a, b, c, x[11], 9);	
			HH(c, d, a, b, x[7], 11);	
			HH(b, c, d, a, x[15], 15);	
		}
	}
	output[0] += a;
	output[1] += b;
	output[2] += c;
	output[3] += d;
}
