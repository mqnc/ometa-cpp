

#include <chrono>
#include <ostream>
#include <string>
#include <iostream>

#include "src/ometa.h"

using ViewTree = ometa::ViewTree<std::string_view>;
auto operator""_S(const char* str, size_t len) {
	return ViewTree(std::string_view(str, len));
}

int main() {

	std::string src = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

	auto v1 = ViewTree(ometa::View(std::string_view(src).substr(5, 10)));
	auto v2 = ViewTree(ometa::View(std::string_view(src).substr(20, 3)));
	auto v3 = ViewTree(ometa::View(std::string_view(src).substr(30, 1)));

	auto conc = v1 + v2 + v3;

	int i = 0;
	for (const auto& view: conc) {
		std::cout << view << " " << std::flush;
		if (i++ > 100) { break; }
	}
	std::cout << conc.size() << std::endl;

	return 0;

}
