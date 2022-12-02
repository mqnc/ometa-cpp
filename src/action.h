
#include "parser.h"

template<typename T, typename F>
auto makeAction(T child, F fn) {

	auto parseFn = [fn]<forward_range TSource>(
		SourceView<TSource> src,
		auto children,
		auto ctx
		) {

		auto result = std::get<0>(children).parse(src, ctx);

		return result.has_value() ?
			match(src, result->next, fn(result->value)) : fail;
	};

	return Parser(parseFn, std::make_tuple(child));
}

template <typename F, typename TChildren, typename A>
auto operator>= (Parser<F, TChildren> parser, A action) {
	return makeAction(parser, action);
}