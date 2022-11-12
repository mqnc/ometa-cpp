
// includes and declarations //

#include <ranges>
#include <algorithm>
#include <deque>
#include <optional>
#include <string>
#include <iostream>
#include <cassert>
#include <vector>
#include <list>
#include <tuple>
#include <variant>

using namespace std::ranges;
using namespace std::string_literals;
struct Empty {};


// SourceView //

template <forward_range TSource>
class SourceView : public std::ranges::view_interface<SourceView<TSource>> {
public:
	using source_type = TSource;
	using iterator_type = decltype(cbegin(std::declval<TSource&>()));
	using sentinel_type = decltype(cend(std::declval<TSource&>()));

	SourceView() = default;
	SourceView(const TSource& src)
		:m_begin(cbegin(src)), m_end(cend(src)) {}
	SourceView(iterator_type begin, sentinel_type end)
		:m_begin(begin), m_end(end) {}
	auto begin() const { return m_begin; }
	auto end() const { return m_end; }
	void shift() { ++m_begin; }
	template<typename T>
	T as() {
		T result;
		for (const auto& item : *this) {
			result.push_back(item);
		}
		return result;
	}
private:
	iterator_type m_begin;
	sentinel_type m_end;
};



// Match //

template<forward_range TSource, typename TValue>
struct Match {
	using source_type = TSource;
	using value_type = TValue;
	SourceView<TSource> next;
	TValue value;
};

template<forward_range TSource, typename TValue>
using MaybeMatch = std::optional<Match<TSource, TValue>>;

inline constexpr auto fail = std::nullopt;

template<forward_range TSource, typename TValue>
inline MaybeMatch<TSource, TValue> match(
	SourceView<TSource> next,
	TValue value
) {
	return Match<TSource, TValue>{next, value};
}


// parser template stuff //u

template<typename F, typename Ret, typename... Args>
std::tuple<Ret, Args...> functionTypesHelper(Ret(F::*)(Args...) const);

template<typename F>
struct ParseArgs {
private:
	using info = decltype(functionTypesHelper(&F::operator()));
public:
	using result_type = std::tuple_element_t<0, info>;
	using arg1_type = std::tuple_element_t<1, info>;
	using arg2_type = std::tuple_element_t<2, info>;
	using source_type = typename arg1_type::source_type;
	using value_type = typename result_type::value_type::value_type;
	using children_type = arg2_type;
};

template<typename T>
struct is_tuple_of_parsers;

template <typename F>
concept parsefn_type =
std::is_same_v<
	typename ParseArgs<F>::result_type,
	MaybeMatch<typename ParseArgs<F>::source_type, typename ParseArgs<F>::value_type>
>
and std::is_same_v<
	typename ParseArgs<F>::arg1_type,
	SourceView<typename ParseArgs<F>::source_type>
>
and is_tuple_of_parsers<typename ParseArgs<F>::arg2_type>::value;

template <parsefn_type F, typename TKind = void>
class Parser;

template <typename>
struct is_parser :public std::false_type {};
template <typename T>
struct is_parser<Parser<T>> :public std::true_type {};
template<typename>
struct is_tuple_of_parsers :public std::false_type {};
template<>
struct is_tuple_of_parsers<std::tuple<>> :public std::true_type {};
template<typename TFirst, typename... TRest>
struct is_tuple_of_parsers<std::tuple<TFirst, TRest...>> {
	static const bool value = is_parser<TFirst>::value
		and is_tuple_of_parsers<std::tuple<TRest...>>::value;
};

template<typename T>
concept parser_type = is_parser<T>::value;
template<typename T>
concept parser_tuple_type = is_tuple_of_parsers<T>::value;


// Parser //

template <parsefn_type F, typename TKind>
class Parser {
	using type_info = ParseArgs<F>;
	F parseFn;
public:

	using Kind = TKind;
	using source_type = typename type_info::source_type;
	using value_type = typename type_info::value_type;
	using children_type = typename type_info::children_type;

	children_type children;

	Parser(F parseFn, children_type children = children_type{}) :
		parseFn{ parseFn }, children{ children } {}

	MaybeMatch<source_type, value_type> parse(SourceView<source_type> src) {
		return parseFn(src, children);
	}

	MaybeMatch<source_type, value_type> parse(source_type src) {
		return parseFn(src, children);
	}

};


// Sequence //

template<parser_type TFirst, typename... TRest>
auto makeSequence(TFirst firstChild, TRest... otherChildren) {

	if constexpr (sizeof...(TRest) == 0) {
		return firstChild;
	}
	else {
		auto combinedRest = makeSequence(otherChildren...);

		using source_type = typename TFirst::source_type;
		static_assert(std::is_same_v <
			source_type,
			typename decltype(combinedRest)::source_type
		>);
		using value_type = std::tuple<
			typename TFirst::value_type,
			typename decltype(combinedRest)::value_type
		>;

		return Parser([](
			SourceView<source_type> src,
			std::tuple <TFirst, decltype(combinedRest)> children
			)->MaybeMatch<source_type, value_type> {

				auto result1 = std::get<0>(children).parse(src);
				if (not result1.has_value()) {
					return fail;
				}

				auto result2 = std::get<1>(children).parse(result1->next);
				if (not result2.has_value()) {
					return fail;
				}

				return match(
					result2->next,
					std::make_tuple(result1->value, result2->value)
				);

			}, std::make_tuple(firstChild, combinedRest)
				);
	}
}


// Choice //

template<parser_type TFirst, typename... TRest>
auto makeChoice(TFirst firstChild, TRest... otherChildren) {

	if constexpr (sizeof...(TRest) == 0) {
		return firstChild;
	}
	else {
		auto combinedRest = makeChoice(otherChildren...);

		using source_type = typename TFirst::source_type;
		static_assert(std::is_same_v <
			source_type,
			typename decltype(combinedRest)::source_type
		>);
		using value_type = std::variant<
			typename TFirst::value_type,
			typename decltype(combinedRest)::value_type
		>;

		auto parseFn = [](
			SourceView<source_type> src,
			std::tuple <TFirst, decltype(combinedRest)> children
			)->MaybeMatch<source_type, value_type> {

				auto result1 = std::get<0>(children).parse(src);
				if (result1.has_value()) {
					value_type v;
					v.template emplace<0>(result1->value);
					return match(result1->next, v);
				}
				auto result2 = std::get<1>(children).parse(src);
				if (result2.has_value()) {
					value_type v;
					v.template emplace<1>(result2->value);
					return match(result2->next, v);
				}
				return fail;
		};

		return Parser(parseFn, std::make_tuple(firstChild, combinedRest)
		);
	}
}


// Repetition //

template<parser_type T>
auto makeRepetition(T child, size_t min, size_t max) {

	using source_type = typename T::source_type;
	using value_type = std::deque<typename T::value_type>;

	return Parser([min, max](
		SourceView<source_type> src, std::tuple<T> child
		)->MaybeMatch<source_type, value_type> {

			value_type results;

			auto next = src;

			for (size_t i = 0; i < max; i++) {
				auto result = std::get<0>(child).parse(next);

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
		}, child
	);
}


// Look Ahead //

enum Polarity { positive, negative };

template<parser_type T>
auto makeLookAhead(T child, Polarity polarity) {

	using source_type = typename T::source_type;
	using value_type = Empty;

	return Parser([polarity](
		SourceView<source_type> src, std::tuple<T> child
		)->MaybeMatch<source_type, value_type> {

			auto result = std::get<0>(child).parse(src);

			if (result.has_value() == (polarity == positive)) {
				return match(src, Empty{});
			}
			else {
				return fail;
			}

		}, child
	);
}


// Literal //

template<forward_range TSource>
auto makeLiteral(auto compare) {
	return Parser([compare](
		SourceView<TSource> src, std::tuple<>
		)->MaybeMatch<TSource, SourceView<TSource>> {

			auto equalUntil = mismatch(src, compare);

			if (equalUntil.in2 == compare.end()) {
				auto next = SourceView<TSource>(equalUntil.in1, src.end());
				auto matched = SourceView<TSource>(src.begin(), equalUntil.in1);
				return match(next, matched);
			}
			else {
				return fail;
			}

		}
	);
}


// Action //

template<parser_type T, typename F>
auto makeAction(T child, F fn) {

	using source_type = typename T::source_type;
	using value_type = std::tuple_element_t<0,
		decltype(functionTypesHelper(&F::operator()))>;

	return Parser([fn](
		SourceView<source_type> src, std::tuple<T> child
		)->MaybeMatch<source_type, value_type> {

			auto result = std::get<0>(child).parse(src);

			if (result.has_value()) {
				return match(result->next, fn(result->value));
			}
			else {
				return fail;
			}

		}, child
	);
}


// Predicate //

template<parser_type T, typename F>
auto makePredicate(T child, F fn) {

	using source_type = typename T::source_type;
	using value_type = typename T::value_type;

	return Parser([fn](
		SourceView<source_type> src, std::tuple<T> child
		)->MaybeMatch<source_type, value_type> {

			auto result = std::get<0>(child).parse(src);

			if (result.has_value() && fn(result->value)) {
				return result;
			}
			else {
				return fail;
			}

		}, child
	);
}


// Syntactic Sugar //

template<parser_type P1, parser_type P2>
auto operator> (P1 parser1, P2 parser2) {
	return makeSequence(parser1, parser2);
}

template<parser_type P1, parser_type P2>
auto operator| (P1 parser1, P2 parser2) {
	return makeChoice(parser1, parser2);
}

template<parser_type P>
auto operator~ (P parser) {
	return makeRepetition(parser, 0, 1);
}
template<parser_type P>
auto operator* (P parser) {
	return makeRepetition(parser, 0, (size_t)-1);
}
template<parser_type P>
auto operator+ (P parser) {
	return makeRepetition(parser, 1, (size_t)-1);
}

template<parser_type P>
auto operator& (P parser) {
	return makeLookAhead(parser, positive);
}
template<parser_type P>
auto operator! (P parser) {
	return makeLookAhead(parser, negative);
}

auto operator""_L(const char* compare, size_t size) {
	return makeLiteral<std::string>(std::string(compare, size));
}

template<parser_type P, typename F>
auto operator>= (P parser, F fn) {
	return makeAction(parser, fn);
}

// main //

int main() {

	auto f1 = [](SourceView<std::vector<int>> v, std::tuple<>)
		-> MaybeMatch<std::vector<int>, int> {

		return std::nullopt;
	};
	Parser p1(f1);


	auto f2 = [](SourceView<std::vector<int>> v, std::tuple<decltype(p1)>)
		-> MaybeMatch<std::vector<int>, int> {

		return std::nullopt;
	};

	Parser p2(f2, std::make_tuple(p1));
	// p.parse({ 1,2,3 });

	auto p3 = "123"_L;

	std::cout << p3.parse("123456"s)->value.as<std::string>() << "\n";;

	auto p4 = makeChoice(p3, p3);

	auto p5 = "123"_L | "456"_L;

	std::cout << std::get<1>(p5.parse("45678")->value).as<std::string>() << "\n";

	auto p6 = "123"_L > "456"_L;

	std::cout << p6.parse("123 456").has_value() << "\n";

	auto p7 = +"a"_L;

	std::cout << p7.parse("aab").has_value() << "\n";

	auto p8 = !"a"_L > &"b"_L > "bc"_L;

	std::cout << p8.parse("bcd").has_value() << "\n";

	auto printer = [](SourceView<std::string> s)->std::string {
		std::cout << s.as<std::string>() << "\n";
		return "ok";
	};

	auto p9 = "123"_L >= printer;

	std::cout << p9.parse("12345")->value << "\n";


	return 0;
}


