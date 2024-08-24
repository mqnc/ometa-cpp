

#include <chrono>
#include <ostream>
#include <string>
#include <iostream>

#include "src/ometa.h"

using Snippet = ometa::Snippet<std::string_view>;
auto operator""_S(const char* str, size_t len) {
	return Snippet(std::string_view(str, len));
}

int main() {

	auto s1 = "Hello "_S;
	auto s2 = "World!"_S;
	std::string s3 = "awa";

	auto v = Snippet{ometa::View<std::string_view>(s3)};

	auto s12 = s1 + s2 + v;

	auto str = s12.collect<std::string>();

	std::cout << str << "\n";
}
