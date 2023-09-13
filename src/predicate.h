#pragma once

#include "parser.h"

namespace ometa {

template <typename T, typename F>
auto predicate(T child, F fn) {

	auto parseFn = [child, fn]<forward_range TSource>
		(
			SourceView<TSource> src,
			auto ctx
		) {
			auto result = child.parseOn(src, ctx);

			return fn(unwrap(result)) ? result : fail;
		};

	return Parser(parseFn);
}


template <typename F, typename P>
auto operator<=(Parser<F> parser, P pred) {
	return predicate(parser, pred);
}

}
