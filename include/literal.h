#pragma once

#include <string>
#include <string_view>
#include <algorithm>

#include "parser.h"
#include "viewtree.h"

namespace ometa {

DECL_DEBUG_TAG(LITERAL, "(literal)", 3);

auto literal(auto compare) {

	auto parseFn = [compare]<forward_range TSource>
		(
			View<TSource> src,
			auto& ctx
		) {
			(void) ctx;

			auto equalUntil = std::ranges::mismatch(src, compare);

			return equalUntil.in2 == compare.end() ? [&] {
					auto next = View<TSource>(equalUntil.in1, src.end());
					auto matched = ViewTree{View<TSource>(src.begin(), equalUntil.in1)};
					return makeMaybeMatch(matched, next);
				}() : fail;
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
