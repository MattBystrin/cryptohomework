#include <gmp.h>
#include <iostream>
#include <fstream>
#include <gmpxx.h>
#include <argparse.hpp>
#include <stdexcept>
#include <chrono>
#include <filesystem>

using bint = mpz_class;
using key  = std::pair<bint, bint>;

inline bint encrypt_blk(bint &blk, key &k)
{
	bint enc;
	mpz_powm(enc.get_mpz_t(), blk.get_mpz_t(),
		 k.first.get_mpz_t(),
		 k.second.get_mpz_t());
	return enc;
}

inline bint decrypt_blk(bint &blk, key &k)
{
	bint dec;
	mpz_powm(dec.get_mpz_t(), blk.get_mpz_t(),
		 k.first.get_mpz_t(),
		 k.second.get_mpz_t());
	return dec;
}

bint random_prime_bits(int L)
{
	gmp_randclass rand(gmp_randinit_default);
	rand.seed(time(NULL));
	bint ret = rand.get_z_bits(L); 
	mpz_setbit(ret.get_mpz_t(), L/2 - 1);
	bint tmp = ret;
	if (mpz_probab_prime_p(ret.get_mpz_t(), 30) < 1)
		mpz_nextprime(ret.get_mpz_t(), tmp.get_mpz_t());
	return ret;
}

void generate_keys(bint e, int L, std::string output)
{
	bint p = random_prime_bits(L/2);
	bint q = random_prime_bits(L/2);
	std::cout << "p: " << p << " len: " << mpz_sizeinbase(p.get_mpz_t(), 2) << std::endl;
	std::cout << "q: " << q << " len: " << mpz_sizeinbase(q.get_mpz_t(), 2) << std::endl;
	bint n = p * q;	
	bint fi = (p - 1) * (q - 1);
	bint g, d, t;
	mpz_gcdext(g.get_mpz_t(), d.get_mpz_t(), t.get_mpz_t(),
		   e.get_mpz_t(), fi.get_mpz_t());
	std::ofstream of(output + ".pub", std::ios::out); 
	if (!of)
		throw std::runtime_error("Can't create key " + output + ".pub");
	of << e << std::endl;
	of << n << std::endl;
	of.flush();
	of.close();
	of.open(output, std::ios::out);
	if (!of)
		throw std::runtime_error("Can't create key " + output);
	of << d << std::endl;
	of << n << std::endl;
	of.flush();
	of.close();
}

std::pair<bint, bint> read_key(std::string keypath)
{
	std::cout << "Loading key: " + keypath << std::endl;
	std::ifstream f(keypath, std::ios::in);
	if (!f)
		throw std::runtime_error("Error opening file");
	std::string tmp;
	f >> tmp;
	bint first(tmp);
	f >> tmp;
	bint second(tmp);
	f.close();
	return {first, second};
}

void encrypt_file(std::string filepath, std::string keypath, std::string output)
{
	auto k = read_key(keypath);
	//Determine key size
	int key_len = mpz_size(k.second.get_mpz_t());
	int blk_len = (key_len % 4 == 0) ? key_len / 4
					 : key_len / 4 + 1;
	//std::cout << "Key len " << key_len << std::endl;
	//std::cout << "Blk len " << blk_len << std::endl;
	mp_limb_t *blk = new mp_limb_t[blk_len];
	std::ifstream ifs(filepath, std::ios::in);
	if (!ifs.is_open())
		throw std::runtime_error("Error opening file");
	std::ofstream ofs(output, std::ios::out);
	if (!ofs.is_open())
		throw std::runtime_error("Error writing output");
	auto start = std::chrono::high_resolution_clock::now();
	while(true) {
		std::fill(blk, blk + blk_len, 0);
		int n = ifs.read((char *)blk, sizeof(mp_limb_t) * blk_len).gcount();
		if (ifs.eof() && n == 0)
			break;
		mpz_t tmp;
		mpz_roinit_n(tmp, blk, blk_len);
		bint in(tmp);
		auto enc = encrypt_blk(in, k);
		ofs << enc << std::endl;
	}
	auto elapsed = std::chrono::high_resolution_clock::now() - start;
	uint64_t e_us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
	std::cout << "Time elapsed for encryption " << e_us << std::endl;
	ifs.close();
	ofs.close();
	delete[] blk;
}

void decrypt_file(std::string filepath, std::string keypath, std::string output)
{
	auto k = read_key(keypath);
	//Determine key size
	//std::cout << "Key " << k.first << std::endl << k.second << std::endl;
	int key_len = mpz_size(k.second.get_mpz_t());
	int blk_len = (key_len % 4 == 0) ? key_len / 4
					 : key_len / 4 + 1;
	//std::cout << "Key len " << key_len << std::endl;
	//std::cout << "Blk len " << blk_len << std::endl;
	std::ifstream ifs(filepath, std::ios::in);
	if (!ifs.is_open())
		throw std::runtime_error("Error opening file");
	std::ofstream ofs(output, std::ios::out);
	if (!ofs.is_open())
		throw std::runtime_error("Error writing output");
	auto start = std::chrono::high_resolution_clock::now();
	while(true) {
		bint tmp;
		ifs >> tmp;
		if (ifs.eof())
			break;
		auto dec = decrypt_blk(tmp, k);
		ofs.write((char *)(mpz_limbs_read(dec.get_mpz_t())), sizeof(mp_limb_t) * blk_len);
	}
	auto elapsed = std::chrono::high_resolution_clock::now() - start;
	uint64_t e_us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
	std::cout << "Time elapsed for decryption " << e_us << std::endl;
	ifs.close();
	ofs.close();
}

void pollard(std::string keypath)
{
	auto k = read_key(keypath);
	bint n = k.second;
	gmp_randclass rand(gmp_randinit_default);
	rand.seed(time(NULL));
	bint res, x, y;
	x = rand.get_z_range(n/2); 
	y = x;
	int i = 0;
	while(res <= 1) {
		y = (y * y + 1) % n;
		std::cout << "Y generated: " << y << std::endl;
		bint delta = abs(x - y);
		std::cout << "Delta: " << delta << std::endl;
		mpz_gcd(res.get_mpz_t(), n.get_mpz_t(), delta.get_mpz_t());
		if (i++ > 0x01 << 12) {
			x = rand.get_z_range(n/2);
			y = x;
			i = 0;
		}
	}
	std::cout << "Found one: " << res << std::endl;
	std::cout << "Second: " << n / res << std::endl;
	
}

void tests(bool regenerate, int num, std::string testdir)
{
	std::filesystem::create_directories(testdir);
	//Generate files for ecnryption
	if (regenerate)  {
		std::cout << "Generating plain files" << std::endl;
		for (int len = 0x01 << 6; len < 0x01 << 16; len <<= 1) {
			std::vector<uint8_t> tmp;
			tmp.resize(len);
			std::fill(tmp.begin(), tmp.end(), 0xAA);
			std::ofstream ofs(testdir+"/plain"+std::to_string(len),
					  std::ios::out | std::ios::binary);
			ofs.write((char *)&tmp[0], len);
			ofs.close();
		}
	}
	for (auto i: {256, 512, 1023}) {
		if (regenerate)
			generate_keys(257, i, testdir + "/key" + std::to_string(i));
		for (int len = 0x01 << 6; len < 0x01 << 16; len <<= 1) {
			std::cout <<  "File " + std::to_string(len) + " ";
			encrypt_file(testdir+"/plain"+std::to_string(len),
				testdir+"/key"+std::to_string(i)+".pub",
				testdir+"/encrypted"+std::to_string(len)+"_"+std::to_string(i));
		}
		for (int len = 0x01 << 6; len < 0x01 << 16; len <<= 1) {
			std::cout <<  "File " + std::to_string(len) + " ";
			decrypt_file(testdir+"/encrypted"+std::to_string(len)+"_"+std::to_string(i),
				testdir+"/key"+std::to_string(i),
				testdir+"/decrypted"+std::to_string(len)+"_"+std::to_string(i));
		}
	}
}

void rsa_test(bint p, bint q, bint e = 257)
{
	bint n = p * q;
	bint fi = (p - 1) * (q - 1);
	bint g, d, t;
	mpz_gcdext(g.get_mpz_t(), d.get_mpz_t(), t.get_mpz_t(),
		   e.get_mpz_t(), fi.get_mpz_t());
	std::srand(time(NULL));
	key pub(e, n);
	key prv(d, n);
	bint plain = std::rand() % n;
	std::cout << "Plain " << plain << std::endl;
	auto enc = encrypt_blk(plain, pub);
	std::cout << "Encrypted " << enc << std::endl;
	auto dec = decrypt_blk(enc, prv);
	std::cout << "Decrypted " << dec << std::endl;
}

void fact(int pL, int qL)
{
	bint p = random_prime_bits(pL);
	bint q = random_prime_bits(qL);
	bint n = p * q;	
	bint s = sqrt(n);
	auto start = std::chrono::high_resolution_clock::now();
	for (bint i = 2; i < s; i++) {
		if (n % i == 0) {
			std::cout << "Found" << i;
			break;
		}
	}
	auto elapsed = std::chrono::high_resolution_clock::now() - start;
	uint64_t e_us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
	std::cout << "Time elapsed " << e_us << std::endl;
}

int main(int argc, char *argv[])
{
	argparse::ArgumentParser parser("task3");
	parser.add_argument("action")
		.help("action to perform: rsa-test,"
					" factorial,"
					" encrypt,"
					" decrypt,"
					" make-key,"
					" pollard");
	parser.add_argument("-f", "--filepath")
		.help("input file");
	parser.add_argument("-o", "--output");
	parser.add_argument("-k", "--key");
	parser.add_argument("-d")
		.help("dir path for test");
	parser.add_argument("-p");
	parser.add_argument("-q");
	parser.add_argument("-L")
		.scan<'i', int>()
		.help("bit lenght for keys");
	parser.add_argument("-e")
		.help("open exponent");

	bint e = 257;

	try {
		parser.parse_args(argc, argv);
	} catch (const std::runtime_error& ex) {
		std::cerr << ex.what() << std::endl;
		std::cerr << parser;
		std::exit(1);
	}

	try {
		std::string action = parser.get<std::string>("action");
		if (action == "make-key") {
			int L = parser.get<int>("-L");
			std::string keypath = parser.get<std::string>("-o");
			generate_keys(257, L, keypath);
		} else if (action == "rsa-test") {
			bint p(parser.get<std::string>("-p"));
			bint q(parser.get<std::string>("-q"));
			rsa_test(p, q);
		} else if (action == "factorial") {
			int L = parser.get<int>("-L");
			for (double r = 0.25; r <= 0.5; r += 0.025) {
				std::cout << "r = " << r << " ";
				fact(r * L, (1 - r) * L);
			}
		} else if (action == "encrypt") {
			std::string keypath = parser.get<std::string>("-k");
			std::string filepath = parser.get<std::string>("-f");
			std::string output = parser.get<std::string>("-o");
			encrypt_file(filepath, keypath, output);
		} else if (action == "decrypt") {
			std::string keypath = parser.get<std::string>("-k");
			std::string filepath = parser.get<std::string>("-f");
			std::string output = parser.get<std::string>("-o");
			decrypt_file(filepath, keypath, output);
		} else if (action == "pollard") {
			std::string keypath = parser.get<std::string>("-k");
			pollard(keypath);
		} else if (action == "tests") {
			tests(true, 10, "tests");
		} else {
			std::cerr << "Invalid action" << std::endl;
		}
	} catch (const std::exception &ex) {
		std::cerr << ex.what() << std::endl;
		std::exit(1);
	}
	return 0;
}
