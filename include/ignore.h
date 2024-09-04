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

std::ostream& operator<<(std::ostream& os, Ignore)
{
	os << "(ignore)";
	return os;
}

template <typename T>
auto ignoreValue(T child) {

	auto parseFn = [child]<forward_range TSource>
		(
			View<TSource> src,
			auto ctx
		) {

			auto result = child.parseOn(src, ctx);

			return result.has_value() ?
				makeMaybeMatch(
					ignore,
					result->next
					)
				: fail;
		};

	return Parser(parseFn);
}

template <typename F>
auto operator~(Parser<F> parser) {
	return ignoreValue(parser);
}

}
