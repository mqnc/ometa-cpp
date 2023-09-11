

#include "src/ometa.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <cmath>

using std::get;

int main(int argc, char *argv[]) {

	auto sum = dummy<std::string_view, int>();

	auto number = CAP(~("+"_L | "-"_L) > +RNG('0', '9'))
		>= [](auto value) -> int {

		std::string s;
		value.match.copyInto(s);
		return std::stoi(s);
	};

	auto atomic = (number AS(num) | "("_L > ref(sum) AS(sum) > ")"_L)
		>= [](auto value) -> int {

		switch (value.index()) {
			case 0: return get<0>(value);
			case 1: return GET(sum);
		}
		throw std::runtime_error("");
	};

	auto power = atomic > *("^"_L > atomic)
		>= [](auto value) -> int {

		int base = get<0>(value);
		auto exponents = get<1>(value);

		int tower = 1;
		for (auto e: exponents) {
			tower = pow(get<1>(e), tower);
		}

		return pow(base, tower);
	};

	auto product = power > *(("*"_L | "/"_L) > power)
		>= [](auto value) -> int {

		int result = get<0>(value);
		auto factors = get<1>(value);

		for (auto f: factors) {
			auto op = get<0>(f);
			auto factor = get<1>(f);

			switch (op.index()) {
				case 0: result *= factor; break;
				case 1: result /= factor; break;
			}
		}

		return result;
	};

	sum = product > *(("+"_L | "-"_L) > product)
		>= [](auto value) -> int {

		int result = get<0>(value);
		auto terms = get<1>(value);

		for (auto t: terms) {
			auto op = get<0>(t);
			auto term = get<1>(t);

			switch (op.index()) {
				case 0: result += term; break;
				case 1: result -= term; break;
			}
		}

		return result;
	};

	if(argc != 2){
		std::cout << "usage: " << argv[0] << " 5+4*3^(2+1)\n";
		return EXIT_FAILURE;
	}

	std::cout << *sum.parse(argv[1]) << "\n";

	return EXIT_SUCCESS;
}
