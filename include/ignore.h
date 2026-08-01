#pragma once

#include "parser.h"

namespace ometa {

struct Ignore {};

constexpr bool operator==(const Ignore, const Ignore) {
	return true;
}

constexpr bool operator==(const auto, const Ignore) {
	return false;
}

Ignore ignore;

constexpr Ignore operator+(const Ignore, const Ignore) {
	return ignore;
}

std::ostream& operator<<(std::ostream& os, Ignore)
{
	os << "(ignore)";
	return os;
}

DECL_DEBUG_TAG(IGNORE, "(ignore)", false);

template <DerivedFromParser T>
auto ignoreValue(T child) {

	auto parseFn = [child]<forward_range TSource>
		(
			View<TSource> src,
			auto& ctx
		) {

			auto result = child.parseOn(src, ctx);

			return result.has_value() ?
				makeMaybeMatch(
					ignore,
					result->next
					)
				: fail;
		};

	return parser<IGNORE>(parseFn);
}

template <DerivedFromParser T>
auto operator~(T parser) {
	return ignoreValue(parser);
}

}
