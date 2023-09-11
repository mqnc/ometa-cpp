#pragma once

#include <type_traits>
#include <variant>
#include "parser.h"

template <bool UseVariant = false, typename... Ts>
auto choice(Ts... children) {

	auto childrenTuple = std::make_tuple(children...);

	auto parseFn = [childrenTuple]<forward_range TSource>
		(
			SourceView<TSource> src,
			auto ctx
		) {

			using common_type = std::conditional_t<
				UseVariant,
				std::variant<decltype(std::declval<Ts>().parseOn(src, ctx)->value)...>,
				decltype(get<0>(childrenTuple).parseOn(src, ctx)->value)
				>;

			using return_type = decltype(makeMaybeMatch(std::declval<common_type>(), src));

			auto recursiveTest = [&]<size_t I = 0>(const auto& self) {
				if constexpr (I >= sizeof...(Ts)) {
					return fail_as<return_type>;
				}
				else {
					auto result = std::get<I>(childrenTuple).parseOn(src, ctx);
					if (result.has_value()) {
						if constexpr (UseVariant) {
							return makeMaybeMatch(
								common_type {std::in_place_index<I>, result->value},
								result->next
							);
						}
						else {
							static_assert(std::is_same_v<common_type, decltype(result->value)>,
								"All generated values in a choice like A | B | C must have the same type."
								" Use A || B || C to generate a variant instead.");
							return result;
						}
					}
					else {
						return self.template operator()<I + 1>(self);
					}
				}
			};

			return recursiveTest(recursiveTest);
		};

	return Parser(parseFn);
}

template <typename F1, typename F2>
auto operator|(Parser<F1> parser1, Parser<F2> parser2) {
	return choice<false>(parser1, parser2);
}

// the PartialChoice wrapper allows chains like
// a || b || c with the parsing result being
// variant<ValA, ValB, ValC> instead of
// variant<variant<ValA, ValB>, ValC>
template <typename F, typename... Ts>
struct PartialChoice: public Parser<F> {
	std::tuple<Ts...> children;

	PartialChoice(Parser<F>&& wrapee, std::tuple<Ts...> children):
		Parser<F>(std::move(wrapee)),
		children {children}
		{};
};

template <typename F1, typename F2>
auto operator||(Parser<F1> parser1, Parser<F2> parser2) {
	return PartialChoice {
		choice<true>(parser1, parser2),
		std::make_tuple(parser1, parser2)
	};
}

template <typename F1, typename... Ts, typename F2>
auto operator||(PartialChoice<F1, Ts...> parser1, Parser<F2> parser2) {
	auto allChildren = std::tuple_cat(
		parser1.children,
		std::make_tuple(parser2)
	);
	return PartialChoice {
		std::apply( [](auto&&... args) {
				return choice<true>(args...);
			}, allChildren),
		allChildren
	};
}



template <class... Args>
std::ostream& operator<<(std::ostream& os, std::variant<Args...> const& v) {
	os << v.index() << "=";
	std::visit([&os](const auto& var) { os << var; }, v);
	return os;
}
