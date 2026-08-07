#pragma once

#include "parser.h"
#include "ignore.h"

namespace ometa {

DECL_DEBUG_TAG(EPSILON, "(epsilon)", false);

auto epsilon() {

	auto parseFn = []<forward_range TSource>
		(
			View<TSource> src,
			auto& ctx
		) {
			(void) ctx;
			return makeMaybeMatch(ignore, src);
		};

	return parser<EPSILON>(parseFn);
}

}