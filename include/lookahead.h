#pragma once

#include "parser.h"
#include "ignore.h"

namespace ometa {

enum Polarity { positive, negative };

template <DerivedFromParser T>
auto lookAhead(T child, Polarity polarity) {

	auto parseFn = [child, polarity]<forward_range TSource>
		(
			View<TSource> src,
			auto& ctx
		) {

			auto result = child.parseOn(src, ctx);

			return result.has_value() == (polarity == positive) ?
				makeMaybeMatch(ignore, src) : fail;
		};

	return Parser(parseFn);
}


template <DerivedFromParser T>
auto operator&(T parser) {
	return lookAhead(parser, positive);
}
template <DerivedFromParser T>
auto operator!(T parser) {
	return lookAhead(parser, negative);
}

}
