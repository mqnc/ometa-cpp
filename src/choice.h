#pragma once

#include <variant>

#include "parser.h"

template<typename... TChildren>
auto makeChoice(TChildren... children) {

	auto parseFn = []<forward_range TSource>(
		SourceView<TSource> src,
		auto children,
		auto ctx
		) {
		using variant_type = std::variant<typename decltype(std::declval<TChildren>().parse(src, ctx))::value_type...>;

		using return_type = decltype(match(src, src, std::declval<variant_type>()));

		auto recursiveTest = [&]<size_t I = 0>(const auto & self) {
			if constexpr (I >= sizeof...(TChildren)) {
				return fail_as<return_type>;
			}
			else {
				auto result = std::get<I>(children).parse(src, ctx);
				if (result.has_value()) {
					return match(
						result->capture,
						result->next,
						variant_type{ std::in_place_index<I>, result.value() }
					);
				}
				else {
					return self.template operator() < I + 1 > (self);
				}
			}
		};

		return recursiveTest(recursiveTest);
	};

	return Parser(parseFn, std::make_tuple(children...));
}

// the PartialChoice wrapper allows chains like
// a | b | c with the parsing result being
// variant<ValA, ValB, ValC> instead of
// variant<variant<ValA, ValB>, ValC>
template <typename F, typename TChildren = std::tuple<>>
class PartialChoice : public Parser<F, TChildren> {
public: PartialChoice(Parser<F, TChildren>&& other) :
	Parser<F, TChildren>(std::move(other)) {};
};

template <typename F1, typename TChildren1, typename F2, typename TChildren2>
auto operator| (Parser<F1, TChildren1> parser1, Parser<F2, TChildren2> parser2) {
	return PartialChoice{ makeChoice(parser1, parser2) };
}

template <typename F1, typename TChildren1, typename F2, typename TChildren2>
auto operator| (PartialChoice<F1, TChildren1> parser1, Parser<F2, TChildren2> parser2) {
	auto allChildren = std::tuple_cat(parser1.children, std::make_tuple(parser2));
	return PartialChoice{
		std::apply([](auto &&... args) {
			// https://stackoverflow.com/a/37100646/3825996
			return makeChoice(args...);
		}, allChildren)
	};
}