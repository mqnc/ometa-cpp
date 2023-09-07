#pragma once

#include "parser.h"

template <typename T, typename F>
auto makePredicate(T child, F fn) {

	auto parseFn = [child, fn]<forward_range TSource>
		(
			SourceView<TSource> src,
			auto ctx
		) {
			auto result = child.parse(src, ctx);

			return fn(result) ? result : fail;
		};

	return Parser(parseFn);
}


template <typename F, typename P>
auto operator<=(Parser<F> parser, P predicate) {
	return makePredicate(parser, predicate);
}
