#pragma once

#include "parser.h"

namespace ometa {

template <typename P, typename F>
struct Predicate: public Parser<F> {
	P fn;

	Predicate(P fn, F parseFn):
		fn {fn},
		Parser<F> {parseFn}
	{}
};

template <typename Until, auto value>
constexpr decltype(value) defer = value;

template <typename P>
auto predicate(P fn) {

	auto parseFn = [fn]<forward_range TSource>
		(
			SourceView<TSource> src,
			auto ctx
		) {
			// we defer the instantiation of this call until TSource is known
			// so the compiler doesn't complain if fn() cannot handle ignore
			// and we actually never call it with ignore
			return fn(defer<TSource, ignore>)
				? makeMaybeMatch(ignore, src) : fail;
		};

	return Predicate(fn, parseFn);
}

template <typename T, typename P, typename F>
auto parameterizedPredicate(T child, Predicate<P, F> pred) {

	auto parseFn = [child, pred]<forward_range TSource>
		(
			SourceView<TSource> src,
			auto ctx
		) {

			auto result = child.parseOn(src, ctx);

			return pred.fn(unwrap(result)) ? result : fail;
		};

	return Parser(parseFn);
}

template <typename F, typename P>
auto operator>(Parser<F> parser, P pred) {
	return parameterizedPredicate(parser, pred);
}

}
