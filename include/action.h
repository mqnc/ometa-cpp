#pragma once

#include "parser.h"
#include "defer.h"

// An action must be a working Parser on its own
// but we must also be able to pipe things into it:
//
// act := {...}; // used standalone, we feed Ignore into it
// paramAct := a act; // now a's value is piped into it

namespace ometa {

DECL_DEBUG_TAG(ACTION, "(action)", false);
DECL_DEBUG_TAG(PARAMETRIC_ACTION, "(parametricAction)", false);

template <typename A, typename F>
struct Action: public Parser<ACTION, F> {
	A fn;

	Action(A fn, F parseFn):
		fn {fn},
		Parser<ACTION, F> {parseFn}
	{}
};

template <typename A>
auto action(A fn) {

	auto parseFn = [fn]<forward_range TSource>
		(
			View<TSource> src,
			auto& ctx
		) {
			// We defer the instantiation of this call until TSource is known
			// so the compiler doesn't complain if fn() cannot handle ignore
			// but we actually never call it with ignore.

			auto noValue = defer<TSource, ignore>;

			constexpr bool actionHasReturn = !std::is_same_v<
				decltype(fn(noValue, ctx)), void>;

			if constexpr(actionHasReturn){
				return makeMaybeMatch(fn(noValue, ctx), src);
			}
			else{
				fn(noValue, ctx);
				return makeMaybeMatch(ignore, src);
			}
		};

	return Action(fn, parseFn);
}

template <DerivedFromParser T, typename A, typename F>
auto parameterizedAction(T child, Action<A, F> act) {

	auto parseFn = [child, act]<forward_range TSource>
		(
			View<TSource> src,
			auto& ctx
		) {

			auto result = child.parseOn(src, ctx);

			constexpr bool actionHasReturn = !std::is_same_v<
				decltype(act.fn(result->value, ctx)), void>;

			if constexpr(actionHasReturn){
				return result.has_value() ?
					makeMaybeMatch(act.fn(result->value, ctx), result->next)
					: fail;
			}
			else{
				return result.has_value() ?
					(act.fn(result->value, ctx), makeMaybeMatch(ignore, result->next))
					: fail;
			}
		};

	return parser<PARAMETRIC_ACTION>(parseFn);
}

template <DerivedFromParser T, typename A, typename F>
auto operator>=(T parser, Action<A, F> act) {
	return parameterizedAction(parser, act);
}

template <DerivedFromParser T, typename Tag, typename A, typename F>
auto operator>=(T parser, RuleWrapper<Tag, Rule<Action<A, F>>> actRule) {
	return rule<Tag>(parameterizedAction(parser, actRule.getBody()));
}

}
