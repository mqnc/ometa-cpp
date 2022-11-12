
// includes and declarations //

#include <ranges>
#include <algorithm>
#include <deque>
#include <optional>
#include <string>
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


// Context //

template<typename TOuter = Empty, typename Tag = Empty, typename TValue = Empty>
class Context {
	const TOuter& outer;
	const TValue value;

	Context(
		const TOuter& outer,
		const TValue& value
	) :outer{ outer }, value{ value } {}

public:

	Context() :outer{ Empty{} }, value{ Empty{} } {}

	template<typename NewTag, typename TNewValue>
	auto add(TNewValue nv) {
		return Context<decltype(*this), NewTag, TNewValue>(
			*this, nv);
	}

	template<typename GetTag>
	auto get() const {
		if constexpr (std::is_same_v<GetTag, Tag>) {
			return value;
		}
		else {
			return outer.template get<GetTag>();
		}
	}

	template<typename, typename, typename>
	friend class Context;
};

template <typename>
struct is_context :public std::false_type {};
template <typename T, typename U, typename V>
struct is_context<Context<T, U, V>> :public std::true_type {};
template<typename T>
concept context_type = is_context<T>::value;


// Match //

template<forward_range TSource, typename TValue>
struct Match {
	SourceView<TSource> next;
	TValue value;
};

template<forward_range TSource, typename TValue>
using MaybeMatch = std::optional<Match<TSource, TValue>>;

inline constexpr auto fail = std::nullopt;

template<typename T>
inline constexpr auto fail_as = static_cast<T>(std::nullopt);

template<forward_range TSource, typename TValue>
inline MaybeMatch<TSource, TValue> match(
	SourceView<TSource> next,
	TValue value
) {
	return Match<TSource, TValue>{next, value};
}


// Parser //

template <typename F, typename TChildren = std::tuple<>>
class Parser {
	F parseFn;
public:

	TChildren children;

	Parser(F parseFn, TChildren children = std::tuple<>{}) :
		parseFn{ parseFn }, children{ children } {}

	auto parse(auto src) {
		return parseFn(SourceView(src), children, Empty{});
	}

	auto parse(auto src, auto ctx) {
		return parseFn(SourceView(src), children, ctx);
	}

	template<forward_range TSource>
	auto parse(SourceView<TSource> src, auto ctx) {
		return parseFn(src, children, ctx);
	}

	//template <typename TSource>
	//using TValue = decltype(std::declval<Parser>().parse(std::declval<TSource>()));

};


// Sequence //

template<typename TFirst, typename... TRest>
auto makeSequence(TFirst firstChild, TRest... otherChildren) {

	if constexpr (sizeof...(TRest) == 0) {
		return firstChild;
	}
	else {
		auto combinedRest = makeSequence(otherChildren...);

		auto parseFn = []<forward_range TSource>(
			SourceView<TSource> src,
			auto children,
			auto ctx
			) {
			using tuple_type = std::tuple<
				decltype(std::get<0>(children).parse(src, ctx)->value),
				decltype(std::get<1>(children).parse(src, ctx)->value)
			>;
			using return_type = decltype(match(src, std::declval<tuple_type>()));

			auto result1 = std::get<0>(children).parse(src, ctx);
			if (not result1.has_value()) {
				return fail_as<return_type>;
			}

			auto result2 = std::get<1>(children).parse(result1->next, ctx);
			if (not result2.has_value()) {
				return fail_as<return_type>;
			}

			return match(
				result2->next,
				std::make_tuple(result1->value, result2->value)
			);

		};

		return Parser(parseFn, std::make_tuple(firstChild, combinedRest));
	}
}


// Choice //

template<typename TFirst, typename... TRest>
auto makeChoice(TFirst firstChild, TRest... otherChildren) {

	if constexpr (sizeof...(TRest) == 0) {
		return firstChild;
	}
	else {
		auto combinedRest = makeChoice(otherChildren...);

		auto parseFn = []<forward_range TSource>(
			SourceView<TSource> src,
			auto children,
			auto ctx
			) {
			using variant_type = std::variant<
				decltype(std::get<0>(children).parse(src, ctx)->value),
				decltype(std::get<1>(children).parse(src, ctx)->value)
			>;
			using return_type = decltype(match(src, std::declval<variant_type>()));

			auto result1 = std::get<0>(children).parse(src, ctx);
			if (result1.has_value()) {
				return match(
					result1->next,
					variant_type{ std::in_place_index<0>, result1->value }
				);
			}

			auto result2 = std::get<1>(children).parse(result1->next, ctx);
			if (result2.has_value()) {
				return match(
					result2->next,
					variant_type{ std::in_place_index<1>, result2->value }
				);
			}

			return fail_as<return_type>;
		};

		return Parser(parseFn, std::make_tuple(firstChild, combinedRest));
	}
}


// Repetition //

template<typename T>
auto makeRepetition(T child, size_t min, size_t max) {

	auto parseFn = [min, max]<forward_range TSource>(
		SourceView<TSource> src,
		auto children,
		auto ctx
		) {

		using return_element_type = decltype(std::get<0>(children).parse(src, ctx));
		std::deque<return_element_type> results{};

		auto next = src;

		for (size_t i = 0; i < max; i++) {
			auto result = std::get<0>(children).parse(next, ctx);

			if (result.has_value()) {
				results.push_back(result->value);
				next = result->next;
			}
			else if (i < min) {
				return fail_as<decltype(match(next, results))>;
			}
			else {
				break;
			}
		}

		return match(next, results);

	};

	return Parser(parseFn, std::make_tuple(child));
}


// Look Ahead //

enum Polarity { positive, negative };

template<typename T>
auto makeLookAhead(T child, Polarity polarity) {

	auto parseFn = [polarity]<forward_range TSource>(
		SourceView<TSource> src,
		auto children,
		auto ctx
		) {

		auto result = std::get<0>(children).parse(src, ctx);

		return result.has_value() == (polarity == positive) ?
			match(src, Empty{}) : fail;
	};

	return Parser(parseFn, std::make_tuple(child));
}


// Literal //

auto makeLiteral(auto compare) {

	auto parseFn = [compare]<forward_range TSource>(
		SourceView<TSource> src,
		auto children,
		auto ctx
		) {
		(void)children;
		(void)ctx;

		auto equalUntil = mismatch(src, compare);

		return equalUntil.in2 == compare.end() ? [&] {
			auto next = SourceView<TSource>(equalUntil.in1, src.end());
			auto matched = SourceView<TSource>(src.begin(), equalUntil.in1);
			return match(next, matched);
		}() : fail;
	};

	return Parser(parseFn, std::make_tuple());
}


// Action //

template<typename T, typename F>
auto makeAction(T child, F fn) {

	auto parseFn = [fn]<forward_range TSource>(
		SourceView<TSource> src,
		auto children,
		auto ctx
		) {

		auto result = std::get<0>(children).parse(src, ctx);

		return result.has_value() ?
			match(result->next, fn(result->value)) : fail;
	};

	return Parser(parseFn, std::make_tuple(child));
}


// Predicate //

template<typename T, typename F>
auto makePredicate(T child, F fn) {

	auto parseFn = [fn]<forward_range TSource>(
		SourceView<TSource> src,
		auto children,
		auto ctx
		) {
		auto result = std::get<0>(children).parse(src, ctx);

		return (result.has_value() && fn(result->value)) ? result : fail;
	};

	return Parser(parseFn, std::make_tuple(child));
}


// Syntactic Sugar //

template <typename F1, typename TChildren1, typename F2, typename TChildren2>
auto operator> (Parser<F1, TChildren1> parser1, Parser<F2, TChildren2> parser2) {
	return makeSequence(parser1, parser2);
}

template <typename F1, typename TChildren1, typename F2, typename TChildren2>
auto operator| (Parser<F1, TChildren1> parser1, Parser<F2, TChildren2> parser2) {
	return makeChoice(parser1, parser2);
}

template <typename F, typename TChildren>
auto operator~ (Parser<F, TChildren> parser) {
	return makeRepetition(parser, 0, 1);
}
template <typename F, typename TChildren>
auto operator* (Parser<F, TChildren> parser) {
	return makeRepetition(parser, 0, (size_t)-1);
}
template <typename F, typename TChildren>
auto operator+ (Parser<F, TChildren> parser) {
	return makeRepetition(parser, 1, (size_t)-1);
}

template <typename F, typename TChildren>
auto operator& (Parser<F, TChildren> parser) {
	return makeLookAhead(parser, positive);
}
template <typename F, typename TChildren>
auto operator! (Parser<F, TChildren> parser) {
	return makeLookAhead(parser, negative);
}

auto operator""_L(const char* compare, size_t size) {
	return makeLiteral<std::string>(std::string(compare, size));
}

template <typename F, typename TChildren, typename A>
auto operator>= (Parser<F, TChildren> parser, A action) {
	return makeAction(parser, action);
}
template <typename F, typename TChildren, typename P>
auto operator<= (Parser<F, TChildren> parser, P predicate) {
	return makePredicate(parser, predicate);
}


// main //

int main() {

	auto a = "a"_L;
	auto b = "b"_L;

	auto ab = a > b;
	auto a_b = a | b;

	auto anda = &a;
	auto nota = !a;
	auto opta = ~a;
	auto zoma = *a;
	auto ooma = +a;

	auto act = a >= [](auto mm){return mm;};
	auto pred = a <= [](auto mm){return true;};

	return 0;
}


