#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <cmath>
#include <optional>

#include <unicode/unistr.h>
#include <vector>
#include "lib/argparse.hpp"

using ustring = icu::UnicodeString;
ustring generate_map(int r = 30);
ustring alfb(" абвгдежзийклмнопрстуфхцчшщъыьэюя");
ustring asort(" оетинавслрдмпкячыуьйгзбшжхюцщэъф");
// Generate key and save it
void make_key(std::string keypath)
{
	ustring map = generate_map();
	std::ofstream k(keypath, std::ios::out);
	std::string tmp;
	map.toUTF8String(tmp);
	k << tmp;
	k.close();
}

void encrypt(std::string filepath, std::string keypath, std::string output)
{
	if (filepath.empty() || keypath.empty())
		throw std::runtime_error("Empty path");
	std::cout << "Start encrypt" << std::endl;
	std::string tmp;
	if (!keypath.empty()) {
		std::ifstream k(keypath, std::ios::in);
		if (!std::getline(k, tmp))
			throw std::runtime_error("Unable to read key");
		k.close();
	} else {
		make_key("key.txt");
	}
	ustring map = ustring::fromUTF8(tmp);
	tmp.clear();
	std::ifstream f(filepath, std::ios::in);
	std::ofstream o(output, std::ios::out);
	while(std::getline(f,tmp)) {
		ustring text = ustring::fromUTF8(tmp);
		text = text.toLower();
		for (int i = 0; i < text.length(); ++i) {
			int index = alfb.indexOf(text[i]);
			if (index == -1)
				continue;
			text.setCharAt(i, map[index]);
		}
		std::string encrypted;
		text.toUTF8String(encrypted);
		o << encrypted << std::endl;
	}
	f.close();
	o.close();
	std::cout << "Done" << std::endl;
}

void decrypt(std::string filepath, std:: string keypath, std::string output)
{
	std::cout << "Start decrypting" << std::endl;
	std::ifstream k(keypath, std::ios::in);
	std::string tmp;
	if (!std::getline(k, tmp))
		throw std::runtime_error("Unable to read key");
	k.close();
	ustring key = ustring::fromUTF8(tmp);
	std::ifstream f(filepath, std::ios::in);
	std::ofstream o(output, std::ios::out);
	while(std::getline(f,tmp)) {
		ustring text = ustring::fromUTF8(tmp);
		text = text.toLower();
		for (int i = 0; i < text.length(); ++i) {
			int index = key.indexOf(text[i]);
			if (index == -1)
				continue;
			text.setCharAt(i, alfb[index]);
		}
		std::string decrypted;
		text.toUTF8String(decrypted);
		o << decrypted << std::endl;
	}
	f.close();
	o.close();
	std::cout << "Done" << std::endl;
}

ustring calculate_table(std::vector<std::pair<char16_t,float>> tbl,
		     const ustring text)
{
	tbl.resize(alfb.length());
	for (int i = 0; i < tbl.size(); ++i) {
		tbl[i].first = alfb[i];
		tbl[i].second = 0;
	}
	for (int i = 0; i < text.length(); i++) {
		int index = alfb.indexOf(text[i]);
		if (index == -1)
			continue;
		++tbl[index].second;
	}
	float n_avg = 1 - tbl[0].second / text.length();
	std::for_each(tbl.begin(), tbl.end(), [&text](std::pair<char16_t, float> &e) { 
		e.second /= text.length();
	});
	std::sort(tbl.begin(), tbl.end(), [](const auto &l, const auto &r) {
		return l.second > r.second;	
	});
	ustring key;
	for (int i = 0; i < alfb.length(); ++i) {
		key.append(tbl[i].first);
	}
	float h_max = std::log2(alfb.length());
	float h = 0;
	for (int i = 0; i < alfb.length(); ++i) {
		if (tbl[i].second == 0)
			continue;
		h -= tbl[i].second * 
		     std::log2(tbl[i].second) ;
	}
	float r = 1 - (h/h_max);
	std::cout << "Entropy: " << h << std::endl
		  << "Redundancy: " << r << std::endl
		  << "Average word length: " << n_avg << std::endl;
	return key;
}

void freq_analysis(std::string filepath, std::string output)
{
	std::cout << "Starting frequency analysis" << std::endl;
	std::ifstream f(filepath, std::ios::in);
	std::stringstream encrypted; 
	encrypted << f.rdbuf();
	ustring text = ustring::fromUTF8(encrypted.str());
	text = text.toLower();
	std::vector<std::pair<char16_t,float>> freq;
	ustring key = calculate_table(freq, text);
	for (int i = 0; i < text.length(); ++i) {
		int index = key.indexOf(text[i]);
		if (index == -1)
			continue;
		text.setCharAt(i, asort[index]);
	}
	std::string decrypted;
	text.toUTF8String(decrypted);
	std::ofstream o(output, std::ios::out);
	o << decrypted << std::endl;
	o.close();
	f.close();
	std::cout << "Done" << std::endl;
}

void swap(ustring &s, int i, int j)
{
	auto tmp = s[i];
	s.setCharAt(i, s[j]);
	s.setCharAt(j, tmp);
}

ustring generate_map(int r)
{
	using namespace icu;
	ustring map = alfb;
	std::srand(time(NULL));
	for (int i = 0; i < r; ++i) {
		int size = map.length();
		swap(map, std::rand() % size, std::rand() % size);
	}
	return map;
}

int main(int argc, char *argv[])
{
	argparse::ArgumentParser parser("task1");
	parser.add_argument("-f", "--filepath")
		.help("file containing plain or encrypted test");
	parser.add_argument("-d", "--decrypt")
		.default_value(false)
		.implicit_value(true);
	parser.add_argument("-e", "--encrypt")
		.default_value(false)
		.implicit_value(true);
	parser.add_argument("-k", "--key")
		.help("file containing key");
	parser.add_argument("-a", "--analysis")
		.default_value(false)
		.implicit_value(true)
		.help("run frequency analysis and try to decrypt text");
	parser.add_argument("-o", "--output")
		.required()
		.help("output file");
	parser.add_argument("--make-key")
		.default_value(false)
		.implicit_value(true)
		.help("generate key");

	std::string file;
	std::string output;
	std::string key;
	bool ecr = false;
	bool dcr = false;
	bool analysis = false;

	try {
		parser.parse_args(argc, argv);
	} catch (const std::runtime_error& ex) {
		std::cerr << ex.what() << std::endl;
		std::cerr << parser;
		std::exit(1);
	}
	try {
		bool mk_key = parser.get<bool>("--make-key");
		if (mk_key) {
			output = parser.get<std::string>("-o");
			make_key(output);
			std::exit(0);
		}
		dcr = parser.get<bool>("-d");
		ecr = parser.get<bool>("-e");
		analysis = parser.get<bool>("-a");
		output = parser.get<std::string>("-o");
		if (dcr && ecr || dcr && analysis || ecr && analysis)
			throw std::runtime_error("Usage [-a|-d|-e]");
		auto f = parser.present<std::string>("-f");
		auto k = parser.present<std::string>("-k");
		if (dcr || ecr || analysis) {
			if (f == std::nullopt)
				throw std::runtime_error("No file provided");
			if (k == std::nullopt && !analysis) 
				throw std::runtime_error("No key provided");
		}
		file = std::move(f.value());
		if (dcr || ecr)
			key = std::move(k.value());
	} catch (const std::runtime_error& ex) {
		std::cerr << ex.what() << std::endl;
		std::exit(1);
	}

	if (ecr)
		encrypt(file, key, output);
	if (dcr)
		decrypt(file, key, output);
	if (analysis)
		freq_analysis(file, output);

	return 0;
}
