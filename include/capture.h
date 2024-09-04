#pragma once

#include "parser.h"
#include "viewtree.h"

namespace ometa {

template <typename T>
auto capture(T child) {

	auto parseFn = [child]<forward_range TSource>
		(
			View<TSource> src,
			auto ctx
		) {

			auto result = child.parseOn(src, ctx);

			return result.has_value() ?
				makeMaybeMatch(
					ViewTree{View<TSource>(
						src.begin(),
						result->next.begin()
						)},
					result->next
					)
				: fail;
		};

	return Parser(parseFn);
}

}
