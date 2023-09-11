#pragma once

#include "parser.h"

enum Polarity { positive, negative };

template <typename T>
auto lookAhead(T child, Polarity polarity) {

	auto parseFn = [child, polarity]<forward_range TSource>
		(
			SourceView<TSource> src,
			auto ctx
		) {

			auto result = child.parseOn(src, ctx);

			return result.has_value() == (polarity == positive) ?
				makeMaybeMatch(empty, src) : fail;
		};

	return Parser(parseFn);
}


template <typename F>
auto operator&(Parser<F> parser) {
	return lookAhead(parser, positive);
}
template <typename F>
auto operator!(Parser<F> parser) {
	return lookAhead(parser, negative);
}
