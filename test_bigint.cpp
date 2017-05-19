#include <iostream>
#include "bigint.h"

int main(int argc, char** argv) {
	std::string a = "828372959747 * 928372973191";
	
	Bigint P = std::string("828372959747");
	Bigint Q = std::string("928372973191");

	std::cout << P * Q << std::endl;

	Bigint two = 2;
	Bigint wa = two.pow(90);

	std::cout << wa - 1 << std::endl;


	return 0;
}