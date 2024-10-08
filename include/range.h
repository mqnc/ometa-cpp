#pragma once

#include "parser.h"
#include "viewtree.h"

namespace ometa {

template <typename T1, typename T2>
auto range(T1 a, T2 b) {

	auto parseFn = [a, b]<forward_range TSource>
		(
			View<TSource> src,
			auto& ctx
		) {
			(void) ctx;
			return a <= *src.begin() && *src.begin() <= b ?
				makeMaybeMatch(
					ViewTree{View<TSource>(
						src.begin(),
						src.begin() + 1
						)},
					src.next()
					)
				: fail;
		};

	return Parser(parseFn);
}

}