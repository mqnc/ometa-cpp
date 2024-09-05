#pragma once

#include "parser.h"
#include "viewtree.h"

namespace ometa {

auto any() {

	auto parseFn = []<forward_range TSource>
		(
			View<TSource> src,
			const auto& ctx
		) {
			(void) ctx;
			return src.begin() != src.end() ?
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