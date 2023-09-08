#pragma once

#include "parser.h"

template <typename T, typename F>
auto makeAction(T child, F fn) {

	auto parseFn = [child, fn]<forward_range TSource>
		(
			SourceView<TSource> src,
			auto ctx
		) {

			auto result = child.parseOn(src, ctx);

			return result.has_value() ?
				match(fn(result->value), result->next)
				: fail;
		};

	return Parser(parseFn);
}

template <typename F, typename A>
auto operator>=(Parser<F> parser, A action) {
	return makeAction(parser, action);
}
