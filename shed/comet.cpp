#include <ranges>
#include <algorithm>
#include <optional>
#include <string>
#include <iostream>
#include <cassert>
#include <vector>
#include <deque>
#include <list>

using namespace std::ranges;
using namespace std::string_literals;
inline constexpr auto fail = std::nullopt;

// Boilerplate

template <class T>
class SourceView;

template<typename T>
struct to_view {
	using type = SourceView<T>;
};
template<typename T>
struct to_view<SourceView<T>> {
	using type = SourceView<T>;
};
template<typename T>
using to_view_t = to_view<T>::type;

template <typename T>
class SourceView : public std::ranges::view_interface<SourceView<T>> {
public:
	SourceView() = default;
	SourceView(const T& src) : m_begin(cbegin(src)), m_end(cend(src)) {}
	auto begin() const { return m_begin; }
	auto end() const { return m_end; }
	void shift() { ++m_begin; }
private:
	decltype(cbegin(std::declval<T&>())) m_begin;
	decltype(cend(std::declval<T&>())) m_end;
};

template<forward_range TNext, typename TValue>
struct Match {
	TNext next;
	TValue value;
};

template<forward_range TInput, typename TValue>
using ParseResult = std::optional<Match<TInput, TValue>>;

template<forward_range TNext, typename TValue>
inline std::optional<Match<TNext, TValue>> match(
	TNext next,
	TValue value
) {
	return Match<TNext, TValue>{next, value};
}

template<typename T>
concept Parser = requires(T p, typename T::input_type src) {
	{ p.parse(src) }
	-> std::same_as<ParseResult<typename T::input_type, typename T::value_type>>;
};

// Sequence

template<Parser TParser1, Parser TParser2>
class Sequence {
	const TParser1& parser1;
	const TParser2& parser2;

public:

	using input_type = typename TParser1::input_type;
	using value_type = std::pair<
		typename TParser1::value_type,
		typename TParser2::value_type
	>;

	Sequence(const TParser1& parser1, const TParser2& parser2) :
		parser1{ parser1 }, parser2{ parser2 } {}

	template<forward_range TInput>
	ParseResult<input_type, value_type> parse(TInput src) const {

		auto result1 = parser1.parse(src);
		if (not result1.has_value()) {
			return fail;
		}

		auto result2 = parser2.parse(result1->next);
		if (not result2.has_value()) {
			return fail;
		}

		return match(result2.next, { result1.value, result2.value });
	}
};

template<Parser P1, Parser P2>
auto operator> (const P1& parser1, const P2& parser2) {
	return Sequence(parser1, parser2);
}

// Choice

template<Parser TParser1, Parser TParser2>
class Choice {
	const TParser1& parser1;
	const TParser2& parser2;

public:

	using input_type = std::common_type_t<
		typename TParser1::input_type,
		typename TParser2::input_type
	>;
	using value_type = std::common_type_t<
		typename TParser1::value_type,
		typename TParser2::value_type
	>;

	Choice(const TParser1& parser1, const TParser2& parser2) :
		parser1{ parser1 }, parser2{ parser2 } {}

	template<forward_range TInput>
	ParseResult<input_type, value_type> parse(TInput src) const {

		auto result1 = parser1.parse(src);
		if (result1.has_value()) {
			return result1;
		}

		auto result2 = parser2.parse(src);
		if (result2.has_value()) {
			return result2;
		}

		return fail;
	}
};

template<Parser P1, Parser P2>
auto operator| (const P1& parser1, const P2& parser2) {
	return Choice(parser1, parser2);
}

// Repetition

template<Parser TParser>
class Repetition {
	const TParser& parser;
	const size_t min;
	const size_t max;

public:

	using input_type = typename TParser::input_type;
	using value_type = std::deque<typename TParser::value_type>;

	Repetition(const TParser& parser, size_t min, size_t max) :
		parser{ parser }, min{ min }, max{ max } {}

	template<forward_range TInput>
	ParseResult<input_type, value_type> parse(TInput src) const {

		value_type results;

		auto next = src;

		for (size_t i = 0; i < max; i++) {
			auto result = parser.parse(next);

			if (result.has_value()) {
				results.push_back(result->value);
				next = result->next;
			}
			else if (i < min) {
				return fail;
			}
			else {
				break;
			}
		}

		return match(next, results);
	}
};

// Literal

template<forward_range TInput, forward_range TCompare>
class Literal {
	TCompare compare;

public:
	using input_type = TInput;
	using value_type = subrange_of_t<TInput>;

	Literal(TCompare compare) : compare{ compare } {}

	ParseResult<input_type, value_type> parse(TInput src) const {
		auto equalUntil = mismatch(src, compare);

		if (equalUntil.in2 == compare.end()) {
			auto next = subrange(equalUntil.in1, src.end());
			auto matched = subrange(src.begin(), equalUntil.in1);
			return match(next, matched);
		}
		else {
			return fail;
		}
	}
};

template<forward_range TCompare>
Literal(TCompare compare)->Literal<subrange_of_t<TCompare>, TCompare>;

auto operator""_L(const char* compare, size_t size) {
	return Literal(std::string(compare, size));
}

int main() {

	// assert(equal(Literal("ab"s).parse("abc"s)->value, "ab"s));

	auto l = Literal<std::list<int>, std::vector<double>>(std::vector<double>{1, 2, 3});
	assert(
		not l
		.parse(std::list<int>{1, 2, 5})
		.has_value()
	);

	auto rule = ("abc"_L | "def"_L) > "gh"_L;

	auto src = ""s;
	auto view = subrange(src.begin(), src.end());

	std::cout << Repetition("a"_L, 1, 2).parse(view).has_value() << "\n";

	return 0;
}
