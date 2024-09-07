#pragma once

#include "parser.h"
#include "defer.h"

namespace ometa {

template <typename C, typename F>
struct ContextModifier: public Parser<F> {
	C fn;

	ContextModifier(C fn, F parseFn):
		fn {fn},
		Parser<F> {parseFn}
	{}
};

template <typename C>
auto contextModifier(C fn) {

	auto parseFn = []<forward_range TSource>
		(
			View<TSource> src,
			const auto& ctx
		) {
			(void) ctx;
			return makeMaybeMatch(ignore, src);
		};

	return ContextModifier(fn, parseFn);
}

template <typename T, typename C, typename F>
auto parametrizedContextModifier(T child, ContextModifier<C, F> mod) {

	auto parseFn = [child, mod]<forward_range TSource>
		(
			View<TSource> src,
			const auto& ctx
		) {
			const auto newCtx = mod.fn(src, ctx);

			return child.parseOn(src, newCtx);
		};

	return Parser(parseFn);
}

template <typename C, typename F1, typename F2>
auto operator<=(ContextModifier<C, F2> mod, Parser<F1> parser) {
	return parametrizedContextModifier(parser, mod);
}


template <typename T1, typename T2>
auto valueToContext(T1 child1, T2 child2) {

	auto parseFn = [child1, child2]<forward_range TSource>
		(
			View<TSource> src,
			const auto& ctx
		) {
			auto result1 = child1.parseOn(src, ctx);
			const auto& newCtx = result1->value;
			auto result2 = child2.parseOn(result1->next, newCtx);
			return result2->value
		};

	return Parser(parseFn);
}

template <typename F1, typename F2>
auto operator<(Parser<F1> parser1, Parser<F2> parser2) {
	return valueToContext(parser1, parser2);
}



}
