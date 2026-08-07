#pragma once

#include <string>
#include <string_view>
#include <algorithm>

#include "parser.h"
#include "viewtree.h"
#include "ignore.h"

namespace ometa {

DECL_DEBUG_TAG(LITERAL, "(literal)", false);

auto literal(auto compare) {

	auto parseFn = [compare]<forward_range TSource>
		(
			View<TSource> src,
			auto& ctx
		) {
			(void) ctx;

			auto equalUntil = std::ranges::mismatch(src, compare);

			return equalUntil.in2 == compare.end() ? 
				makeMaybeMatch(
					ignore,
					View<TSource>(equalUntil.in1, src.end())
				)
				: fail;
		};

	return parser<LITERAL>(parseFn);
}

auto literal(const char* compare) {
	return literal(std::string_view(compare));
}

auto operator""_lit_(const char* compare, size_t size) {
	return literal(std::string_view(compare, size));
}

}
