#include <iostream>
#include <string>
#include <vector>
#include <tuple>

template <typename NEXT, typename VAL>
struct Result {
	bool match;
	NEXT next;
	VAL value;
};

template <auto L>
struct Literal {
	static auto parse(auto src) {
		if (src[0] == L) {
			return Result {true, src + 1, std::string("(") + std::string{L} + ")"};
		}
		else {
			return Result {false, src, std::string("-")};
		}
	}
};

template <typename R1, typename R2>
struct Sequence {
	static auto parse(auto src) {
		auto m1 = R1::parse(src);
		if (!m1.match) {
			return Result {false, src, std::string("-")};
		}
		else {
			auto m2 = R2::parse(m1.next);
			if (!m2.match) {
				return Result {false, src, std::string("-")};
			}
			else {
				return Result {true, m2.next, m1.value + m2.value};
			}
		}
	}
};

template <typename R1, typename R2>
struct Choice {
	static auto parse(auto src) {
		auto m1 = R1::parse(src);
		if (m1.match) {
			return m1;
		}
		else {
			return R2::parse(src);
		}
	}
};

int main() {

	const char* source = "aaaaab";

	struct AB;

	struct AB:public Choice<Sequence<Literal<'a'>, AB>, Literal<'b'>>{};

	std::cout << AB::parse(source).value << "\n";

	return 0;
}
