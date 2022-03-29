#include <iostream>
#include <fstream>
#include <cmath>

#include "lib/argparse.hpp"
#include <string.h>
#include "generator.hpp"

std::vector<int> polynom = {84, 71};

std::vector<uint8_t> load_key(std::string keypath);

float serial_test(const std::vector<uint8_t> &seq, int k)
{
	std::vector<double> freq;
	freq.resize(0x01 << k);
	uint8_t blk = 0;
	int blk_i = 0;
	double N = 0;
	bit_adapter<uint8_t> bitseq(seq);
	for (int i = 0; i < bitseq.size(); ++i) {
		blk |= bitseq[i];
		blk <<= 1;
		blk_i++;
		if (blk_i == k) {
			freq[blk >> 1]++;
			N++;
			blk_i = 0;
			blk = 0;
		}
	}
	/* Debug */
	for (auto f = freq.begin(); f != freq.end(); ++f) {
		*f /= N;
		std::cout << *f << std::endl;
	}
	/* Calculate statistic */
	double V = 0;
	double nt = 1.0 / freq.size();
	for (auto i = freq.begin(); i != freq.end(); ++i) {
		V += (*i - nt) * (*i - nt) / nt;
	}
	std::cout << "Statistic " << V << std::endl;
	return 0;
}

int get_quintet_type(const std::array<int, 10> &q)
{
	bool dflg = false;
	bool tflg = false;
	for (auto &i: q) {
		switch(i) {
		case 2:
			if (dflg)
				return 2;
			else
				dflg = true;
			break;
		case 3:
			tflg = true;
			break;
		case 4:
			return 5;
		case 5:
			return 6;
		default:
			break;
		}
	}
	if (dflg && tflg)
		return 4;
	if (tflg)
		return 3;
	if (dflg)
		return 1;
	return 0;
}

float poker_test(const std::vector<uint32_t> &seq)
{
	std::array<int, 10> quintet;
	std::array<double, 7> freq;
	quintet.fill(0);
	freq.fill(0);
	int qi = 0; 
	for (auto i: seq) {
		int tmp = (static_cast<double>(i) / 0xFFFFFFFF) * 10;
		quintet[tmp]++;
		qi++;
		if (qi == 5) {
			qi = 0;
			int kek = get_quintet_type(quintet);
			freq[kek]++;
			quintet.fill(0);
		}
	}
	std::array<double, 7> p = {
		9 * 8 * 7 * 6 / 10000.0,
		9 * 8 * 7 / 1000.0,
		9 * 8 * 15 / 10000.0,
		9 * 8 / 1000.0,
		9 / 1000.0,
		9 * 5 / 10000.0,
		1 / 10000.0
	};
	double V = 0;
	for (int i = 0; i < p.size(); ++i) {
		std::cout << freq[i] << std::endl;
		freq[i] /= seq.size();
		V += (freq[i] - p[i]) * (freq[i] - p[i]) / p[i];
	}
	std::cout << "Statistic " << V << std::endl;
	return 0;

}

float correlation_test(std::vector<uint8_t> &seq, int k)
{
	std::cout << "k = " << k ;
	bit_adapter<uint8_t> bitseq(seq);
	const double N = bitseq.size();
	double m1 = 0; // i
	double m2 = 0; // i+k
	double d1 = 0;
	double d2 = 0;
	//Calculate expected value
	for (int i = 0; i < N - k; ++i) {
		m1 += bitseq[i];
		m2 += bitseq[i + k];
	}
	m1 /= N - k;
	m2 /= N - k;
	for (int i = 0; i < N - k; ++i) {
		d1 += (bitseq[i] - m1) * (bitseq[i] - m1);
		d2 += (bitseq[i + k] - m2) * (bitseq[i + k] - m2);
	}
	d1 /= N - k - 1;
	d2 /= N - k - 1;
	// Calculate correlation
	double R = 0;
	for (int i = 0; i < N -  k; ++i) {
		R += (bitseq[i] - m1) * (bitseq[i + k] - m2);
	}
	R /= (N - k) * sqrt(d1 * d2);
	double Rcr = 1/(N+1) + 2/(N-2)*sqrt(N*(N-3)/(N+1));
	std::cout << "	R = " << R << std::endl;
	std::cout << "	Rcr = " << Rcr << std::endl;
	return 0;
}


std::vector<uint8_t> read_file(std::string path)
{
	std::vector<uint8_t> seq;
	std::ifstream f(path, std::ios::in | std::ios::binary);
	if (!f)
		throw std::runtime_error("Error opening file");
	while(true) {
		uint8_t byte;
		f.read((char *)&byte, sizeof(byte));
		if (f.eof())
			break;
		seq.push_back(byte);
	}
	f.close();
	return seq;
}

std::vector<uint32_t> u8to32(std::vector<uint8_t> src)
{
	std::cout << "Src size " << src.size() << std::endl;
	int len = src.size() % 4 == 0 ? src.size() / 4
				      : src.size() / 4 + 1;
	std::vector<uint32_t> res;
	res.resize(len);
	std::fill(res.begin(), res.end(), 0);
	memcpy((uint8_t *)&res[0], &src[0], src.size());
	std::cout << "Result len " << res.size() << std::endl;
	return res;
}

void test(std::string type,
	  std::optional<int> n,
	  std::optional<std::string> keypath,
	  std::optional<std::string> filepath)
{
	std::cout << "Starting test" << std::endl;
	std::vector<uint8_t> key;
	std::vector<uint8_t> seq;
	if (filepath != std::nullopt)
		seq = read_file(filepath.value());
	if (keypath != std::nullopt)
		key = load_key(keypath.value());
	generator<uint8_t> gen(polynom);
	if (!key.empty())
		gen.load(key);
	if (seq.empty())
		for (int i = 0; i < n; ++i)
			seq.push_back(gen.generate<uint8_t>());

	if (type == "serial") {
		std::cout << "Serial test" << std::endl;
		serial_test(seq, 2);
	} else if (type == "poker") {
		std::cout << "Poker test" << std::endl;
		auto adap = u8to32(seq);
		poker_test(adap);
	} else if (type == "corr") {
		std::cout << "Correlation test" << std::endl;
		correlation_test(seq, 4);
	} else
		throw std::runtime_error("Invalid test type");
	std::cout << "Done" << std::endl;
}

std::vector<uint8_t> load_key(std::string keypath)
{
	std::cout << "Loading key: " + keypath << std::endl;
	std::vector<uint8_t> key;
	std::ifstream f(keypath, std::ios::in | std::ios::binary);
	int bitlen = *std::max_element(polynom.begin(), polynom.end());
	int size = bitlen % (sizeof(uint8_t) * 8) == 0 ? bitlen / (sizeof(uint8_t) * 8)
						 : bitlen / (sizeof(uint8_t) * 8) + 1;
	for (int i = 0; i < size; ++i) {
		uint8_t byte = 0;
		f.read((char *)&byte, sizeof(byte));
		key.push_back(byte);
	}
	f.close();
	return key;
}

void crypt(std::string keypath,
	     std::string filepath,
	     std::string output) 
{
	std::cout << "Preforming encryption/decryption" << std::endl;
	auto key = load_key(keypath);
	std::cout << "key size " << key.size() << std::endl;
	for (auto &kek : key)
		printf("0x%02X\n", kek);
	generator<uint8_t> gen(polynom);
	gen.load(key);
	std::ifstream ifs(filepath, std::ios::in | std::ios::binary);
	std::ofstream ofs(output, std::ios::out | std::ios::binary);
	if (!ifs.is_open() || !ofs.is_open())
		throw std::runtime_error("Error opening files");
	while(true) {
		uint8_t byte;
		ifs.read((char *)&byte, sizeof(byte));
		if (ifs.eof())
			break;
		byte ^= gen.generate<uint8_t>();
		ofs.write((char *)&byte, sizeof(byte));
	}
	ifs.close();
	ofs.close();
	std::cout << "Done" << std::endl;
}

void make_key(std::string output)
{
	std::cout << "Making key" << std::endl;
	std::srand(time(NULL));
	std::ofstream f(output, std::ios::out | std::ios::binary);
	int bitlen = *std::max_element(polynom.begin(), polynom.end());
	int dsize = sizeof(uint8_t) * 8;
	int size = bitlen % dsize == 0 ? bitlen / dsize
				       : bitlen / dsize + 1;
	for (int i = 0; i < size; ++i) {
		uint8_t block = std::rand();
		f.write((const char *)&block, sizeof(block));
		if (f.bad())
			std::cout << "Error writing key" << std::endl;
	}
	f.close();
	std::cout << "Done" << std::endl;
}

int main(int argc, char *argv[])
{
	argparse::ArgumentParser parser("task2");
	parser.add_argument("action")
		.required()
		.help("action to perform: encrypt, decrypt, test, make-key, periodic");
	parser.add_argument("-f", "--filepath")
		.help("input file");
	parser.add_argument("-o", "--output");
	parser.add_argument("-k", "--key")
		.help("key for encryption/decryption");
	parser.add_argument("-t", "--type")
		.help("test type");
	parser.add_argument("-n")
		.scan<'i', int>()
		.help("num for test");
	std::string action;
	std::string type;
	try {
		parser.parse_args(argc, argv);
	} catch (const std::runtime_error& ex) {
		std::cerr << ex.what() << std::endl;
		std::cerr << parser;
		std::exit(1);
	}
	try {
		// Output of sequence
		action = parser.get("action");
		if (action == "decrypt" || action == "encrypt") {
			auto k = parser.present<std::string>("-k");
			auto f = parser.present<std::string>("-f");
			auto o = parser.present<std::string>("-o");
			if (k == std::nullopt)
				throw std::runtime_error("No key provided");
			if (f == std::nullopt)
				throw std::runtime_error("No file provided");
			std::string default_name;
			if (action == "encrypt")
				default_name = "encrypted";
			else
				default_name = "decrypted";
			if (o == std::nullopt)
				crypt(k.value(), f.value(), default_name);
			else
				crypt(k.value(), f.value(), o.value());
		} else if (action == "test") {
			auto t = parser.present<std::string>("-t");	
			if (t == std::nullopt)
				throw std::runtime_error("No type provided");	
			test(std::move(t.value()),
			     parser.present<int>("-n"),
			     parser.present<std::string>("-k"),
			     parser.present<std::string>("-f"));
		} else if (action == "make-key") {
			auto o = parser.present<std::string>("-o");
			if (o == std::nullopt)
				make_key("./key");
			else
				make_key(std::move(o.value()));
		} else if (action == "periodic") {
			auto f = parser.present<std::string>("-f");
			if (f == std::nullopt)
				throw std::runtime_error("No input file");
			auto seq = read_file(f.value());
			uint8_t key[2];
			key[0] = std::rand();
			key[1] = std::rand();
			for (int i = 0; i < seq.size(); ++i) {
				seq[i] ^= key[i % 2];
			}
			for (int i = 0; i < 32; ++i) {
				correlation_test(seq, i);
			}
			
		} else
			throw std::runtime_error("Invalid action. See --help");
	} catch (const std::runtime_error& ex) {
		std::cerr << ex.what() << std::endl;
		std::exit(1);
	}
	return 0;
}
