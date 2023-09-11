#pragma once

#include "parser.h"
#include "tree.h"

template <typename T1, typename T2>
auto makeSequence(T1 child1, T2 child2) {

	auto parseFn = [child1, child2]<forward_range TSource>
		(
			SourceView<TSource> src,
			auto ctx
		) {

			using ReturnType = decltype(
				makeMaybeMatch(
					join(
						child1.parseOn(src, ctx)->value,
						child2.parseOn(src, ctx)->value
						),
					src
					)
			);

			auto result1 = child1.parseOn(src, ctx);
			if (result1.has_value()) {
				auto result2 = child2.parseOn(result1->next, ctx);
				if (result2.has_value()) {
					return makeMaybeMatch(
						join(
							result1->value,
							result2->value
							),
						result2->next
					);
				}
			}

			return fail_as<ReturnType>;
		};

	return Parser(parseFn);
}

template <typename F1, typename F2>
auto operator>(Parser<F1> parser1, Parser<F2> parser2) {
	return makeSequence(parser1, parser2);
}
