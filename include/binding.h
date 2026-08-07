#pragma once

#include "parser.h"
#include "ignore.h"

namespace ometa{

enum class Binding { bound, unbound };

// unbound predicates/actions are bound to stub
DECL_DEBUG_TAG(STUB, "(stub)", false);

auto stub() {

	auto parseFn = []<forward_range TSource>
		(
			View<TSource> src,
			auto& ctx
		) {
			(void) ctx;
			return makeMaybeMatch(ignore, src);
		};

	return parser<STUB>(parseFn);
}

}