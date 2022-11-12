#include <iostream>
#include <string>
#include <vector>
#include <tuple>

void frobnicate(auto fn, std::vector<auto> v) {
	fn(v[0]);
}

void frobnicate(auto fn, auto x) {
	fn(x);
}


template<typename... Ts>
class ChoiceValue :public std::variant<Ts...> {};

template<typename>
struct is_ChoiceValue : std::false_type {};

template<typename... Ts>
struct is_ChoiceValue<ChoiceValue<Ts...>> : std::true_type {};


int main() {

	auto fn = [](auto... param) {
		std::cout << param << "\n";
	};

	frobnicate(fn, std::vector<double>{1, 2});
	frobnicate(fn, 3);


	return 0;
}
