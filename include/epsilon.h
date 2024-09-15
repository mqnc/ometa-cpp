#pragma once

#include "parser.h"
#include "empty.h"

namespace ometa {

auto epsilon() {

	auto parseFn = []<forward_range TSource>
		(
			View<TSource> src,
			auto& ctx
		) {
			(void) ctx;
			return makeMaybeMatch(empty, src);
		};

	return Parser(parseFn);
}

}