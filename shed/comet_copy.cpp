#include <ranges>
#include <algorithm>
#include <optional>
#include <string>
#include <iostream>
#include <cassert>
#include <vector>
#include <list>

using namespace std::ranges;
using namespace std::string_literals;


struct Parser {};
template <typename T>
concept ParserType = std::derived_from<T, Parser>;


template<forward_range TIn>
struct subrange_of {
	using type = subrange<decltype(std::declval<TIn>().begin())>;
};
template<forward_range TIn>
using subrange_of_t = subrange_of<TIn>::type;



template<forward_range TInSubrange, typename TValue>
struct Match {
	TInSubrange next;
	TValue value;
};


template<forward_range TCompare>
class Literal :public Parser {
	TCompare compare;

public:
	template<forward_range TIn>
	using TSuccess = Match<subrange_of_t<TIn>, subrange_of_t<TIn>>;

	Literal(TCompare compare) : compare{ compare } {}

	template<forward_range TIn>
	std::optional<TSuccess<TIn>> parse(TIn src) const {
		auto equalUntil = mismatch(src, compare);

		if (equalUntil.in2 == compare.end()) {
			auto next = subrange(equalUntil.in1, src.end());
			auto matched = subrange(src.begin(), equalUntil.in1);
			return TSuccess<TIn>{ next, matched };
		}
		else {
			return std::nullopt;
		}
	}
};
auto operator""_L(const char* compare, size_t size) {
	return Literal(std::string(compare, size));
}


template<ParserType TParser1, ParserType TParser2>
class Choice :public Parser {
	const TParser1& parser1;
	const TParser2& parser2;

public:

	Choice(const TParser1& parser1, const TParser2& parser2) :
		parser1{ parser1 }, parser2{ parser2 } {}

	template<forward_range TIn>
	auto parse(TIn src) const {

		auto result1 = parser1.parse(src);
		if (result1.has_value()) {
			return result1;
		}

		auto result2 = parser2.parse(src);
		if (result2.has_value()) {
			return result2;
		}

		std::common_type_t<decltype(result1), decltype(result2)> fail = std::nullopt;
		return fail;
	}
};

template<ParserType P1, ParserType P2>
auto operator| (const P1& parser1, const P2& parser2) {
	return Choice(parser1, parser2);
}




int main() {

	assert(equal(Literal("ab"s).parse("abc"s)->value, "ab"s));

	assert(
		not Literal(std::vector<double>{1, 2, 3})
		.parse(std::list<int>{1, 2, 5})
		.has_value()
	);

	auto rule = "abc"_L | "def"_L;
	rule.parse("def"s)->value;

	return 0;
}