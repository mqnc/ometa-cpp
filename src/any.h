#pragma once

#include "parser.h"

auto any() {

	auto parseFn = []<forward_range TSource>
		(
			SourceView<TSource> src,
			auto ctx
		) {
			(void) ctx;
			return src.begin() != src.end() ?
				makeMaybeMatch(
					SourceView<TSource>(
						src.begin(),
						src.begin() + 1
						),
					src.next()
					)
				: fail;
		};

	return Parser(parseFn);
}
