#pragma once

#include "parser.h"
#include "viewtree.h"
#include "ignore.h"

namespace ometa {

DECL_DEBUG_TAG(ANY, "(any)", false);

auto any() {

	auto parseFn = []<forward_range TSource>
		(
			View<TSource> src,
			auto& ctx
		) {
			(void) ctx;
			auto it = src.begin();
			return it != src.end()
				? makeMaybeMatch(ignore, src.next()) : fail;
		};

	return parser<ANY>(parseFn);
}

}