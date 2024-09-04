#pragma once

#include <type_traits>
#include <variant>
#include "parser.h"

namespace ometa {

template <typename T1, typename T2>
auto choice(T1 child1, T2 child2) {

	auto parseFn = [child1, child2]<forward_range TSource>
		(
			View<TSource> src,
			auto ctx
		) {
			using Result1Type = decltype(child1.parseOn(src, ctx));
			using Result2Type = decltype(child2.parseOn(src, ctx));

			static_assert(std::is_same_v<Result1Type, Result2Type>);

			auto result1 = child1.parseOn(src, ctx);
			return result1.has_value() ?
				result1 : child2.parseOn(src, ctx);
		};

	return Parser(parseFn);
}

template <typename F1, typename F2>
auto operator|(Parser<F1> parser1, Parser<F2> parser2) {
	return choice(parser1, parser2);
}

}
