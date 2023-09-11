#pragma once

#include "parser.h"

template <typename T1, typename T2>
auto range(T1 a, T2 b) {

	auto parseFn = [a, b]<forward_range TSource>
		(
			SourceView<TSource> src,
			auto ctx
		) {
			(void) ctx;
			return a <= *src.begin() && *src.begin() <= b ?
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
