#pragma once

#include "parser.h"

namespace ometa {

template <typename A, typename F>
struct Action: public Parser<F> {
	A fn;

	Action(A fn, F parseFn):
		fn {fn},
		Parser<F> {parseFn}
	{}
};

template <typename A>
auto action(A fn) {

	auto parseFn = [fn]<forward_range TSource>
		(
			SourceView<TSource> src,
			auto ctx
		) {
			return makeMaybeMatch(fn(ignore), src);
		};

	return Action(fn, parseFn);
}

template <typename T, typename A, typename F>
auto parameterizedAction(T child, Action<A, F> act) {

	auto parseFn = [child, act]<forward_range TSource>
		(
			SourceView<TSource> src,
			auto ctx
		) {

			auto result = child.parseOn(src, ctx);

			return result.has_value() ?
				makeMaybeMatch(act.fn(result->value), result->next)
				: fail;
		};

	return Parser(parseFn);
}

template <typename F, typename A>
auto operator>=(Parser<F> parser, A act) {
	return parameterizedAction(parser, act);
}

}
