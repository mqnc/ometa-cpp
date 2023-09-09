#pragma once

#include "parser.h"
#include "stringliteral.h"

template <typename... Ts>
auto makeSequence(Ts... children) {

	auto childrenTuple = std::make_tuple(children...);

	auto parseFn = [childrenTuple]<forward_range TSource>
		(
			SourceView<TSource> src,
			auto ctx
		) {
			using tuple_type = std::tuple<
				decltype(std::declval<Ts>().parseOn(src, ctx)->value)...>;

			tuple_type matches;
			auto next = src;
			bool success = true;

			auto recursiveSteps = [&]<size_t I = 0>(const auto& self, auto ctx) {
				if constexpr (I < sizeof...(Ts)) {
					auto result = std::get<I>(childrenTuple).parseOn(next, ctx);
					if (result.has_value()) {
						std::get<I>(matches) = result->value;
						next = result->next;
						auto newCtx = []<StringLiteral Tag, typename TVal, forward_range TSrc>
							(Match<Tag, TVal, TSrc> match, auto ctx) {
								if constexpr (Tag == StringLiteral("")) {
									return ctx;
								}
								else {
									return ctx.template add<Tag>(match.value);
								}
							}(*result, ctx);
						self.template operator()<I + 1>(self, newCtx);
					}
					else {
						success = false;
					}
				}
			};

			recursiveSteps(recursiveSteps, ctx);

			return success ? makeMaybeMatch(matches, next) : fail;
		};

	return Parser(parseFn);
}

// the PartialSequence wrapper allows chains like
// a > b > c with the parsing result being
// tuple<ValA, ValB, ValC> instead of
// tuple<tuple<ValA, ValB>, ValC>
template <typename F, typename... Ts>
struct PartialSequence: public Parser<F> {
	std::tuple<Ts...> children;

	PartialSequence(Parser<F>&& wrapee, std::tuple<Ts...> children):
		Parser<F>(std::move(wrapee)),
		children {children}
		{};
};

template <typename F1, typename F2>
auto operator>(Parser<F1> parser1, Parser<F2> parser2) {
	return PartialSequence {
		makeSequence(parser1, parser2),
		std::make_tuple(parser1, parser2)
	};
}

template <typename F1, typename... Ts, typename F2>
auto operator>(PartialSequence<F1, Ts...> parser1, Parser<F2> parser2) {
	auto allChildren = std::tuple_cat(
		parser1.children,
		std::make_tuple(parser2)
	);
	return PartialSequence {
		std::apply( [](auto&&... args) {
				return makeSequence(args...);
			}, allChildren),
		allChildren
	};
}


template <class... Args>
std::ostream& operator<<(std::ostream& os, std::tuple<Args...> const& t) {
	os << "(";
	bool first = true;
	std::apply( [&os, &first](auto&&... args) {
			auto print = [&](auto&& val) {
				if (!first) { os << ","; }
				(os << " " << val);
				first = false;
			};
			(print(args), ...);
		}, t);
	os << " )";
	return os;
}
