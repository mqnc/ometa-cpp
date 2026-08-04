#pragma once

#include <type_traits>
#include <variant>
#include "parser.h"

namespace ometa {

DECL_DEBUG_TAG(CHOICE, "(choice)", false);

template <DerivedFromParser T1, DerivedFromParser T2>
auto choice(T1 child1, T2 child2) {

	auto parseFn = [child1, child2]<forward_range TSource>
		(
			View<TSource> src,
			auto& ctx
		) {
			using Result1Type = decltype(child1.parseOn(src, ctx));
			using Result2Type = decltype(child2.parseOn(src, ctx));

			static_assert(std::is_same_v<Result1Type, Result2Type>, "both children of a Choice must return same semantic value type");

			auto result1 = child1.parseOn(src, ctx);
			return result1.has_value() ?
				result1 : child2.parseOn(src, ctx);
		};

	return parser<CHOICE>(parseFn);
}

template <DerivedFromParser T1, DerivedFromParser T2>
auto operator|(T1 parser1, T2 parser2) {
	return choice(parser1, parser2);
}

}
