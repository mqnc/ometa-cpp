#pragma once

#include "parser.h"
#include "rule.h"
#include "defer.h"

// A predicate must be a working Parser on its own
// but we must also be able to pipe things into it:
//
// pred := {? true}; // used standalone, we feed Ignore into it
// paramPred := a pred; // now a's value is piped into it

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
			auto& ctx
		) {
			// we defer the instantiation of this call until TSource is known
			// so the compiler doesn't complain if fn() cannot handle ignore
			// but we actually never call it with ignore
			return fn(defer<TSource, ignore>, ctx)
				? makeMaybeMatch(ignore, src) : fail;
		};

	return Predicate(fn, parseFn);
}

template <DerivedFromParser T, typename P, typename F>
auto parameterizedPredicate(T child, Predicate<P, F> pred) {

	auto parseFn = [child, pred]<forward_range TSource>
		(
			View<TSource> src,
			auto& ctx
		) {

			auto result = child.parseOn(src, ctx);

			return pred.fn(unwrap(result), ctx) ? result : fail;
		};

	return Parser(parseFn);
}

template <DerivedFromParser T, typename P, typename F>
auto operator>(T parser, Predicate<P, F> pred) {
	return parameterizedPredicate(parser, pred);
}

template <DerivedFromParser T, Tag tag, typename P, typename F>
auto operator>(T parser, RuleWrapper<Rule<tag, Predicate<P, F>>> predRule) {
	return rule<tag>(parameterizedPredicate(parser, predRule.getBody()));
}

}
