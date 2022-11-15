
#include "parser.h"

template<typename T, typename F>
auto makePredicate(T child, F fn) {

	auto parseFn = [fn]<forward_range TSource>(
		SourceView<TSource> src,
		auto children,
		auto ctx
		) {
		auto result = std::get<0>(children).parse(src, ctx);

		return fn(result) ? result : fail;
	};

	return Parser(parseFn, std::make_tuple(child));
}


template <typename F, typename TChildren, typename P>
auto operator<= (Parser<F, TChildren> parser, P predicate) {
	return makePredicate(parser, predicate);
}