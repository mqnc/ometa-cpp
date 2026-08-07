#pragma once

#include "parser.h"
#include "viewtree.h"
#include "ignore.h"

namespace ometa {

DECL_DEBUG_TAG(RANGE, "(range)", false);

template <typename T1, typename T2>
auto range(T1 a, T2 b) {

	auto parseFn = [a, b]<forward_range TSource>
		(
			View<TSource> src,
			auto& ctx
		) {
			(void) ctx;
			auto it = src.begin();
			return (it != src.end() && a <= *it && *it <= b)
				? makeMaybeMatch(ignore, src.next()) : fail;
		};

	return parser<RANGE>(parseFn);
}

}