#pragma once

#include "parser.h"

namespace ometa {

struct Ignore {
	constexpr bool operator==(const Ignore other) const {
		return true;
	}
};

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
			SourceView<TSource> src,
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
