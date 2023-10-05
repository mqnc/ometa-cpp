#pragma once

#include "parser.h"
#include "defer.h"

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
			// we defer the instantiation of this call until TSource is known
			// so the compiler doesn't complain if fn() cannot handle ignore
			// but we actually never call it with ignore
			return makeMaybeMatch(fn(defer<TSource, ignore>), src);
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
