#include <iostream>
#include <gmpxx.h>
#include <argparse.hpp>

mpz_class encrypt_blk(mpz_class blk, mpz_class e, mpz_class n)
{
	mpz_class enc;
	mpz_powm(enc.get_mpz_t(), blk.get_mpz_t(), e.get_mpz_t(), n.get_mpz_t());
	return enc;
}

mpz_class decrypt_blk(mpz_class blk, mpz_class d, mpz_class n)
{
	mpz_class dec;
	mpz_powm(dec.get_mpz_t(), blk.get_mpz_t(), d.get_mpz_t(), n.get_mpz_t());
	return dec;
}


std::pair<mpz_class, mpz_class> generate_key()
{
	return {0, 0};
}

int main(int argc, char *argv[])
{
	argparse::ArgumentParser parser("task3");
	//parser.add_argument("action")
	//	.help("action to perform: encrypt, decrypt, test, make-key");
	parser.add_argument("-f", "--filepath")
		.help("input file");
	parser.add_argument("-o", "--output");
	parser.add_argument("-p");
	parser.add_argument("-q");
	parser.add_argument("-e")
		.help("exponent");

	mpz_class p = 11,
		  q = 13,
		  e = 17,
		  g;
	mpz_class d,t;

	try {
		parser.parse_args(argc, argv);
	} catch (const std::runtime_error& ex) {
		std::cerr << ex.what() << std::endl;
		std::cerr << parser;
		std::exit(1);
	}

	try {
		/*auto p_opt = parser.present<std::string>("-p");
		if (p_opt != std::nullopt)
			p = p_opt.value();
		auto q_opt = parser.present<std::string>("-q");
		if (q_opt != std::nullopt)
			q = q_opt.value();
		auto e_opt = parser.present<std::string>("-e");
		if (e_opt != std::nullopt)
			e = e_opt.value();
		*/
		mpz_class n = p * q;	
		mpz_class fi = (p - 1) * (q - 1);
		std::cout << "n = " << n << std::endl;
		std::cout << "fi = " << fi << std::endl;
		mpz_gcdext(g.get_mpz_t(), d.get_mpz_t(), t.get_mpz_t(),
			   e.get_mpz_t(), fi.get_mpz_t());
		std::cout << "g = " << g << std::endl;
		std::cout << "d = " << d << std::endl;
		std::cout << "t = " << t << std::endl;
		mpz_class pl = 10;
		std::cout << "Plain " << pl << std::endl;
		auto cip = encrypt_blk(pl, e,  n);
		std::cout << "Cipher " << cip << std::endl;
		auto dec = decrypt_blk(cip, d, n); 
		std::cout << "Plain " << pl << std::endl;

	} catch (const std::runtime_error& ex) {
		std::cerr << ex.what() << std::endl;
		std::exit(1);
	}
	return 0;
}
