#pragma once

#include "parser.h"

enum Polarity { positive, negative };

template <typename T>
auto makeLookAhead(T child, Polarity polarity) {

	auto parseFn = [child, polarity]<forward_range TSource>
		(
			SourceView<TSource> src,
			auto ctx
		) {

			auto result = child.parse(src, ctx);

			return result.has_value() == (polarity == positive) ?
				match(src, src, empty) : fail;
		};

	return Parser(parseFn);
}


template <typename F>
auto operator&(Parser<F> parser) {
	return makeLookAhead(parser, positive);
}
template <typename F>
auto operator!(Parser<F> parser) {
	return makeLookAhead(parser, negative);
}
