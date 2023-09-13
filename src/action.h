#pragma once

#include "parser.h"

namespace ometa {

template <typename T, typename F>
auto action(T child, F fn) {

	auto parseFn = [child, fn]<forward_range TSource>
		(
			SourceView<TSource> src,
			auto ctx
		) {

			auto result = child.parseOn(src, ctx);

			return result.has_value() ?
				makeMaybeMatch(fn(result->value), result->next)
				: fail;
		};

	return Parser(parseFn);
}

template <typename F, typename A>
auto operator>=(Parser<F> parser, A fn) {
	return action(parser, fn);
}

}
