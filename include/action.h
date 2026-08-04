#pragma once

#include "parser.h"
#include "rule.h"
#include "logger.h"
#include "binding.h"

// An action must be a working Parser on its own
// but we must also be able to pipe things into it:
//
// act := {...}; // unbound use - we feed an empty into it
// paramAct := a act; // now bound - a's value is piped into it

namespace ometa {

DECL_DEBUG_TAG(UNBOUND_ACTION, "(unbound action)", false);
DECL_DEBUG_TAG(ACTION, "(action)", false);

template <typename A, typename F>
struct UnboundAction: public Parser<UNBOUND_ACTION, F> {
	A fn;

	UnboundAction(A fn, F parseFn):
		fn {fn},
		Parser<UNBOUND_ACTION, F> {parseFn}
	{}
};

template <Binding bind, DerivedFromParser T, typename A, typename FDiscarded>
auto action(T child, UnboundAction<A, FDiscarded> act) {

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

	if constexpr(bind == Binding::bound){
		return parser<ACTION>(parseFn);
	}
	else{
		return UnboundAction(act.fn, parseFn);
	}
}

template <typename A>
auto action(A fn) {
	return action<Binding::unbound>(stub(), UnboundAction{fn, empty});
}

template <DerivedFromParser T, typename A, typename F>
auto operator>=(T parser, UnboundAction<A, F> act) {
	return action<Binding::bound>(parser, act);
}

}
