#include <iostream>
#include <fstream>
#include "scanner.hpp"
#include "parser.hpp"

int main(int argc, char *argv[])
{
	if (argc < 2)
		return 1;

	std::ifstream ifs(argv[1], std::ios::in);
	test::scanner s(ifs, std::cout);
	test::parser p(s);
	auto res = p.parse();

	return res;
}
