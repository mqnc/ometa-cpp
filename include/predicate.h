#pragma once

#include "parser.h"
#include "defer.h"

namespace ometa {

template <typename P, typename F>
struct Predicate: public Parser<F> {
	P fn;

	Predicate(P fn, F parseFn):
		fn {fn},
		Parser<F> {parseFn}
	{}
};

template <typename P>
auto predicate(P fn) {

	auto parseFn = [fn]<forward_range TSource>
		(
			View<TSource> src,
			const auto& ctx
		) {
			// we defer the instantiation of this call until TSource is known
			// so the compiler doesn't complain if fn() cannot handle ignore
			// but we actually never call it with ignore
			return fn(defer<TSource, ignore>)
				? makeMaybeMatch(ignore, src) : fail;
		};

	return Predicate(fn, parseFn);
}

template <typename T, typename P, typename F>
auto parameterizedPredicate(T child, Predicate<P, F> pred) {

	auto parseFn = [child, pred]<forward_range TSource>
		(
			View<TSource> src,
			const auto& ctx
		) {

			auto result = child.parseOn(src, ctx);

			return pred.fn(unwrap(result)) ? result : fail;
		};

	return Parser(parseFn);
}

template <typename F1, typename P, typename F2>
auto operator>(Parser<F1> parser, Predicate<P, F2> pred) {
	return parameterizedPredicate(parser, pred);
}

}
