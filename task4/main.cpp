#include <iostream>
#include <argparse.hpp>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <chrono>

#include "md4.hpp"
#include "bit_adapter.hpp"

template <typename T>
void print_array(std::string name, const T& container)
{
	std::cout << name << ": ";
	printf("0x");
	for (auto &it: container)
		printf("%0*X", sizeof(it) * 2, it);
	std::cout << std::endl;
}

void bitdiff(std::vector<uint8_t> input,
		int rounds = 3,
		std::array<uint32_t, 4> start = {
			0x67452301,
			0xEFCDAB89,
			0x98BADCFE,
			0x10325476
		})
{
	std::vector<uint8_t> copy = input;
	md4 ihash(start, rounds);
	ihash.update(input);
	auto ih = ihash.finish();
	bit_adapter<uint8_t> bit_cpy(copy);
	for (int i = 0; i < input.size() * 8; ++i) {
		bit_cpy.set_at(i, !bit_cpy[i]);
		md4 chash(start, rounds);
		chash.update(copy);
		auto ch = chash.finish();
		std::vector<uint8_t> diff;
		for (int i = 0; i < ch.size(); ++i) {
			diff.push_back(ih[i] ^ ch[i]);
		}
		bit_adapter<uint8_t> bit_diff(diff);
		int diff_cnt = 0;
		for (int i = 0; i < bit_diff.size(); ++i) {
			if (bit_diff[i])
				diff_cnt++;
		}
		print_array("Source", ih);
		std::cout << "Diff bits in plain " << i + 1 << std::endl; 
		print_array("Changed", ch);
		std::cout << "Diff bits in hash " << diff_cnt << std::endl; 
	}

}

void find_collision(int k)
{
	std::cout << "Start calculating collision" << std::endl;
	/* Hash is the key, string - value */
	std::map<std::vector<uint8_t>, std::vector<uint8_t>> hashes;
	std::srand(time(NULL));
	int i = 0;
	while(true) {
		std::vector<uint8_t> in;
		in.resize(10);
		std::generate(in.begin(), in.end(), [](){ return rand();});
		md4 md;
		md.update(in);
		auto hash = md.finish();
		std::vector<uint8_t> hash_head(hash.begin(), hash.begin() + k);
		auto f = hashes.find(hash_head);
		if (f != hashes.end()) {
			std::cout << "Collision found" << std::endl;
			print_array("Hash", hash_head);
			print_array("String 1", in);
			print_array("String 2", f->second);
			std::cout << "Iterations passed: " << i << std::endl;
			break;
		} else {
			hashes.insert(std::pair(hash_head,in));
		}
		if (i % 10000 == 0)
			std::cout << i << " variants scanned" << std::endl;
		i++;
	}
}

void pre_image(int k, std::array<uint8_t, 16> hash)
{
	std::cout << "Start calculating pre-image" << std::endl;
	/* Hash is the key, string - value */
	std::srand(time(NULL));
	int i = 0;
	while(true) {
		std::vector<uint8_t> in;
		in.resize(rand() % 100);
		std::generate(in.begin(), in.end(), [](){ return rand();});
		md4 md;
		md.update(in);
		auto rhash = md.finish();
		/* Compare hashes heads */
		if ([&](int k) -> bool {
			for (int i = 0; i < k; ++i)
				if (hash[i] != rhash[i])
				return false;
			return true;
		}(k)) {
			std::cout << "Scanned " << i << std::endl;
			print_array("Pre-image", in);
			break;
		}
		if (i % 10000 == 0)
			std::cout << i << " variants scanned" << std::endl;
		i++;
	}
}

std::array<uint8_t, 16> hash_from_str(std::string str)
{
	std::array<uint8_t, 16> ret;
	std::fill(ret.begin(), ret.end(), 0);
	if (str.empty())
		return ret;
	// npos
	// begin
	// else
	int base = 10;
	auto prefix = str.find("0x");
	if (prefix == 0) {
		str = str.substr(2);
		base = 16;
	}
	if (prefix != 0 && prefix != std::string::npos)
		throw std::runtime_error("Bad hash format");
	if (str.size() % 2 == 1)
		str = "0" + str;
	int j = ret.size() - 1; 
	for (int i = str.size() - 2; i >= 0; i -= 2)
		ret[j--] = std::stoi(str.substr(i, 2), nullptr, base); 
	return ret;
}

int main(int argc, char *argv[])
{
	argparse::ArgumentParser parser("task4");
	parser.add_argument("action")
		.help("action to perform: hash,"
		      "bitdiff,"
		      "collision,"
		      "pre-img,"
		      "time"
		      );
	parser.add_argument("-f", "--filepath")
		.help("input file");
	parser.add_argument("-s", "--string")
		.help("string for hashing");
	parser.add_argument("-k")
		.scan<'i', int>()
		.help("first k bytes for collision");
	parser.add_argument("-h")
		.help("hash for finding pre-image");
	parser.add_argument("--consts")
		.nargs(4)
		.scan<'x', uint32_t>()
		.help("list of init constants a,b,c,d for md4");
	parser.add_argument("-r", "--rounds")
		.scan<'i', int>()
		.help("md4 rounds number: 1 to 3");

	try {
		parser.parse_args(argc, argv);
	} catch (const std::runtime_error& ex) {
		std::cerr << ex.what() << std::endl;
		std::cerr << parser;
		std::exit(1);
	}

	try {
		std::string action = parser.get("action");
		if (action == "hash") {
			std::string str;
			auto s = parser.present<std::string>("-s");
			if (s != std::nullopt)
				str = s.value();
			else
				throw std::runtime_error("No string provided");
			std::vector<uint8_t> in(str.begin(), str.end());
			md4 hash;
			hash.update(in);
			auto h = hash.finish();
			print_array("", h);
		} else if (action == "bitdiff") {
			std::string str;
			auto s = parser.present<std::string>("-s");
			if (s != std::nullopt)
				str = s.value();
			else
				throw std::runtime_error("No string provided");
			std::vector<uint8_t> in(str.begin(), str.end());
			auto consts_opt = parser.present<std::vector<uint32_t>>("--consts");
			auto rounds_opt = parser.present<int>("-r");
			if (consts_opt != std::nullopt) {
				std::vector<uint32_t> tmp;
				tmp = std::move(consts_opt.value());
				std::array<uint32_t,4> consts;
				std::copy(tmp.begin(), tmp.begin() + 4, consts.begin());
				bitdiff(in, 3, consts);
			} else if (rounds_opt != std::nullopt) {
				bitdiff(in, rounds_opt.value());
			} else
				bitdiff(in);
		} else if (action == "collision") {
			int k = parser.get<int>("-k");
			find_collision(k);
		} else if (action == "pre-img") {
			std::string hash_str = parser.get<std::string>("-h");
			int k = parser.get<int>("-k");
			pre_image(k, hash_from_str(hash_str));
		} else if (action == "time") {
			for (unsigned long int i = 1024; i < 1024 * 100; i+= 1024) {
				std::vector<uint8_t> input;
				input.resize(i);
				std::fill(input.begin(), input.end(), 0xAA);
				md4 hcont;
				auto start = std::chrono::high_resolution_clock::now();
				hcont.update(input);
				hcont.finish();
				auto finish = std::chrono::high_resolution_clock::now() - start;
				uint64_t elapsed = std::chrono::duration_cast<std::chrono::microseconds>(finish).count();
				std::cout << i << " Time " << elapsed << std::endl;
			}
		} else {
			throw std::runtime_error("Ivalid action");
		}
	} catch (const std::runtime_error& ex) {
		std::cerr << ex.what() << std::endl;
		std::exit(1);
	}
	return 0;
}
