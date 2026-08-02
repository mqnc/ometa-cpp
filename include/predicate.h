#pragma once

#include "parser.h"
#include "defer.h"
#include "rule.h"
#include "logger.h"

// A predicate must be a working Parser on its own
// but we must also be able to pipe things into it:
//
// pred := {? true}; // used standalone, we feed Ignore into it
// paramPred := a pred; // now a's value is piped into it

namespace ometa {

DECL_DEBUG_TAG(PREDICATE, "(predicate)", false);
DECL_DEBUG_TAG(PARAMETRIC_PREDICATE, "(parametricPredicate)", false);

template <typename P, typename F>
struct Predicate: public Parser<PREDICATE, F> {
	P fn;

	Predicate(P fn, F parseFn):
		fn {fn},
		Parser<PREDICATE, F> {parseFn}
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

			return result.has_value() && pred.fn(result->value, ctx) ? result : fail;
		};

	return parser<PARAMETRIC_PREDICATE>(parseFn);
}

// abc > predicate -> parametricPredicate(abc)
template <DerivedFromParser T, typename P, typename F>
auto operator>(T parser, Predicate<P, F> pred) {
	return parameterizedPredicate(parser, pred);
}

// uglily, we also need to support
// abc > rule(predicate) -> rule(parametricPredicate(abc))
template <DerivedFromParser T, typename Tag, typename P, typename F>
auto operator>(T parser, RuleWrapper<Tag, WrapperFn<Predicate<P, F>>> predRule) {
	return rule<Tag>(
		parameterizedPredicate(
			parser,
			predRule.getChild()
		)
	);
}

// atrociously, we also need to support
// abc > logger(rule(predicate)) -> logger(rule(parametricPredicate(abc)))
template <DerivedFromParser T, typename Tag, typename P, typename F>
auto operator>(T parser, LoggerWrapper<LoggerFn<RuleWrapper<Tag, WrapperFn<Predicate<P, F>>>>> logPredRule) {
	return logger(
		logPredRule.getLevel(),
		rule<Tag>(
			parameterizedPredicate(
				parser,
				logPredRule.getChild().getChild()
			)
		)
	);
}

}
