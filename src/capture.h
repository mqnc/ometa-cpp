#pragma once

#include "parser.h"
#include "snippet.h"

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
					Snippet{View<TSource>(
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
