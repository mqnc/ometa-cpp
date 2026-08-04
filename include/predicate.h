#pragma once

#include "parser.h"
#include "defer.h"
#include "rule.h"
#include "logger.h"
#include "binding.h"

// A predicate must be a working Parser on its own
// but we must also be able to pipe things into it:
//
// pred := {? true}; // unbound use - we feed empty into it
// paramPred := a pred; // now bound - a's value is piped into it

namespace ometa {

DECL_DEBUG_TAG(UNBOUND_PREDICATE, "(unbound predicate)", false);
DECL_DEBUG_TAG(PREDICATE, "(predicate)", false);

template <typename P, typename F>
struct UnboundPredicate: public Parser<UNBOUND_PREDICATE, F> {
	P fn;

	UnboundPredicate(P fn, F parseFn):
		fn {fn},
		Parser<UNBOUND_PREDICATE, F> {parseFn}
	{}
};

template <Binding bind, DerivedFromParser T, typename P, typename FDiscarded>
auto predicate(T child, UnboundPredicate<P, FDiscarded> pred) {

	auto parseFn = [child, pred]<forward_range TSource>
		(
			View<TSource> src,
			auto& ctx
		) {

			auto result = child.parseOn(src, ctx);

			return result.has_value() && pred.fn(result->value, ctx) ? result : fail;
		};

	if constexpr(bind == Binding::bound){
		return parser<PREDICATE>(parseFn);
	}
	else{
		return UnboundPredicate(pred.fn, parseFn);
	}
}

template <typename P>
auto predicate(P fn) {
	return predicate<Binding::unbound>(stub(), UnboundPredicate{fn, empty});
}

template <DerivedFromParser T, typename P, typename F>
auto operator>=(T parser, UnboundPredicate<P, F> pred) {
	return predicate<Binding::bound>(parser, pred);
}

}
