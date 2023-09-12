

#include "src/ometa.h"

#include <cassert>
#include <iostream>
#include <cmath>

int main(int argc, char* argv[]) {

	auto sum = dummy<std::string_view, int>();

	auto number = capture(~("+"_L | "-"_L) > +range('0', '9')) >=
		[](auto value) {
			return std::stoi(value.template copyAs<std::string>());
		};

	auto atomic = number
		| "("_L > ref(sum) > ")"_L >= ometa::select<1>;

	auto power = atomic > *("^"_L > atomic) >=
		ometa::rfold([](auto prev, auto op, auto temp) {
			std::cout << prev << op << temp << "\n";
			assert(op == "^");
			return int(pow(prev, temp));
		});

	auto product = power > *(("*"_L | "/"_L) > power) >=
		ometa::lfold([](auto temp, auto op, auto next) {
			if (op == "*") { return temp * next; }
			else { assert(op == "/"); return temp / next; }
		});

	sum = product > *(("+"_L | "-"_L) > product) >=
		ometa::lfold([](auto temp, auto op, auto next) {
			if (op == "+") { return temp + next; }
			else { assert(op == "-"); return temp - next; }
		});

	assert(*sum.parse("5+4*3^(2+1-0)") == 113);
	assert(*sum.parse("4^3^2") == 262144);

	if (argc != 2) {
		std::cout << "usage: " << argv[0] << " 5+4*3^(2+1-0)\n";
		return EXIT_FAILURE;
	}

	std::cout << *sum.parse(argv[1]) << "\n";

	return EXIT_SUCCESS;
}
