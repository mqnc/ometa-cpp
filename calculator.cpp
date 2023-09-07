// #include "src/ometa.h"

// #include <cassert>
// #include <iostream>
// #include <sstream>
// #include <cmath>

// using std::get;

// int main(int argc, char *argv[]) {

// 	auto sum = makeDummy<std::string_view, int>();

// 	auto number = ~("+"_L | "-"_L) > +("0"_L | "1"_L | "2"_L | "3"_L | "4"_L | "5"_L | "6"_L | "7"_L | "8"_L | "9"_L)
// 		>= [](auto matched) -> int {

// 		std::string s;
// 		matched->capture.copyInto(s);
// 		return std::stoi(s);
// 	};

// 	auto atomic = (number | "("_L > REF(sum) > ")"_L)
// 		>= [](auto matched) -> int {

// 		switch (matched->value.index()) {
// 			case 0: return get<0>(matched->value).value;
// 			case 1: return get<1>(get<1>(matched->value).value).value;
// 		}
// 		throw std::runtime_error("");
// 	};

// 	auto power = atomic > *("^"_L > atomic)
// 		>= [](auto matched) -> int {

// 		int base = get<0>(matched->value).value;
// 		auto exponents = get<1>(matched->value).value;

// 		int tower = 1;
// 		for (auto e: exponents) {
// 			tower = pow(get<1>(e.value).value, tower);
// 		}

// 		return pow(base, tower);
// 	};

// 	auto product = power > *(("*"_L | "/"_L) > power)
// 		>= [](auto matched) -> int {

// 		int result = get<0>(matched->value).value;
// 		auto factors = get<1>(matched->value).value;

// 		for (auto f: factors) {
// 			auto op = get<0>(f.value).value;
// 			auto factor = get<1>(f.value).value;

// 			switch (op.index()) {
// 				case 0: result *= factor; break;
// 				case 1: result /= factor; break;
// 			}
// 		}

// 		return result;
// 	};

// 	sum = product > *(("+"_L | "-"_L) > product)
// 		>= [](auto matched) -> int {

// 		int result = get<0>(matched->value).value;
// 		auto terms = get<1>(matched->value).value;

// 		for (auto t: terms) {
// 			auto op = get<0>(t.value).value;
// 			auto term = get<1>(t.value).value;

// 			switch (op.index()) {
// 				case 0: result += term; break;
// 				case 1: result -= term; break;
// 			}
// 		}

// 		return result;
// 	};

// 	if(argc != 2){
// 		std::cout << "usage: " << argv[0] << " 5+4*3^(2+1)\n";
// 		return EXIT_FAILURE;
// 	}

// 	std::cout << sum.parse(argv[1])->value << "\n";

// 	return EXIT_SUCCESS;
// }
