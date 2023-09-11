#pragma once

#include <string>
#include <string_view>
#include <algorithm>

#include "parser.h"

auto literal(auto compare) {

	auto parseFn = [compare]<forward_range TSource>
		(
			SourceView<TSource> src,
			auto ctx
		) {
			(void) ctx;

			auto equalUntil = std::ranges::mismatch(src, compare);

			return equalUntil.in2 == compare.end() ? [&] {
					auto next = SourceView<TSource>(equalUntil.in1, src.end());
					auto matched = SourceView<TSource>(src.begin(), equalUntil.in1);
					return makeMaybeMatch(matched, next);
				}() : fail;
		};

	return Parser(parseFn);
}

auto literal(const char* compare) {
	return literal(std::string_view(compare));
}

auto operator""_L(const char* compare, size_t size) {
	return literal<std::string>(std::string(compare, size));
}
