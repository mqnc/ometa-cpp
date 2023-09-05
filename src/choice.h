#pragma once

#include <variant>
#include "parser.h"

template <typename... Ts>
auto makeChoice(Ts... children) {

	auto childrenTuple = std::make_tuple(children...);

	auto parseFn = [childrenTuple]<forward_range TSource>
		(
			SourceView<TSource> src,
			auto ctx
		) {
			using variant_type = std::variant<
				typename decltype(std::declval<Ts>().parse(src, ctx))::value_type...>;

			using return_type = decltype(match(src, src, std::declval<variant_type>()));

			auto recursiveTest = [&]<size_t I = 0>(const auto& self) {
				if constexpr (I >= sizeof...(Ts)) {
					return fail_as<return_type>;
				}
				else {
					auto result = std::get<I>(childrenTuple).parse(src, ctx);
					if (result.has_value()) {
						return match(
							result->capture,
							result->next,
							variant_type {std::in_place_index<I>, result.value()}
						);
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

// the PartialChoice wrapper allows chains like
// a | b | c with the parsing result being
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
auto operator|(Parser<F1> parser1, Parser<F2> parser2) {
	return PartialChoice {
		makeChoice(parser1, parser2),
		std::make_tuple(parser1, parser2)
	};
}

template <typename F1, typename... Ts, typename F2>
auto operator|(PartialChoice<F1, Ts...> parser1, Parser<F2> parser2) {
	auto allChildren = std::tuple_cat(
		parser1.children,
		std::make_tuple(parser2)
	);
	return PartialChoice {
		std::apply( [](auto&&... args) {
				return makeChoice(args...);
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
