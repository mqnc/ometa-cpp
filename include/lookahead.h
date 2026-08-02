#pragma once

#include "parser.h"
#include "ignore.h"

namespace ometa {

DECL_DEBUG_TAG(LOOK_AHEAD, "(lookAhead)", false);
DECL_DEBUG_TAG(NOT_FOLLOWED_BY, "(notFollowedBy)", false);

enum Polarity { positive, negative };

template <Polarity polarity, DerivedFromParser T>
auto lookAhead(T child) {

	auto parseFn = [child]<forward_range TSource>
		(
			View<TSource> src,
			auto& ctx
		) {
			if constexpr (has_backup_method<decltype(ctx)>()) {
				auto backup = ctx.backup();
				auto result = child.parseOn(src, ctx);
				ctx.backtrack(backup);
				return result.has_value() == (polarity == positive) ?
					makeMaybeMatch(ignore, src) : fail;
			}
			else{
				auto result = child.parseOn(src, ctx);
				return result.has_value() == (polarity == positive) ?
					makeMaybeMatch(ignore, src) : fail;
			}
		};

	if constexpr(polarity == positive){
		return parser<LOOK_AHEAD>(parseFn);
	}
	else{
		return parser<NOT_FOLLOWED_BY>(parseFn);
	}
}


template <DerivedFromParser T>
auto operator&(T parser) {
	return lookAhead<positive>(parser);
}
template <DerivedFromParser T>
auto operator!(T parser) {
	return lookAhead<negative>(parser);
}

}
