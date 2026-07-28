#pragma once

#include "parser.h"
#include "valuetree.h"

namespace ometa {

template <DerivedFromParser T1, DerivedFromParser T2>
auto sequence(T1 child1, T2 child2) {

	auto parseFn = [child1, child2]<forward_range TSource>
		(
			View<TSource> src,
			auto& ctx
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

template <DerivedFromParser T1, DerivedFromParser T2>
auto operator>(T1 parser1, T2 parser2) {
	return sequence(parser1, parser2);
}

}