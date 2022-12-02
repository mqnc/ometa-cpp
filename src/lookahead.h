#pragma once

#include "parser.h"

enum Polarity { positive, negative };

template<typename T>
auto makeLookAhead(T child, Polarity polarity) {

	auto parseFn = [polarity]<forward_range TSource>(
		SourceView<TSource> src,
		auto children,
		auto ctx
		) {

		auto result = std::get<0>(children).parse(src, ctx);

		return result.has_value() == (polarity == positive) ?
			match(src, src, empty) : fail;
	};

	return Parser(parseFn, std::make_tuple(child));
}


template <typename F, typename TChildren>
auto operator& (Parser<F, TChildren> parser) {
	return makeLookAhead(parser, positive);
}
template <typename F, typename TChildren>
auto operator! (Parser<F, TChildren> parser) {
	return makeLookAhead(parser, negative);
}
