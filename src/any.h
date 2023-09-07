#pragma once

#include "parser.h"

auto makeAny() {

	auto parseFn = []<forward_range TSource>
		(
			SourceView<TSource> src,
			auto ctx
		) {
			(void) ctx;
			return src.begin() != src.end() ?
				match(
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

#define ANY makeAny()
