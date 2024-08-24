#pragma once

#include <ostream>

#include "empty.h"
#include "view.h"
#include "tag.h"

namespace ometa {

template <
	typename TValue,
	forward_range TSource
	>
struct Match {
	TValue value;
	View<TSource> next;
};

template <
	typename TValue,
	forward_range TSource
	>
using MaybeMatch = std::optional<Match<TValue, TSource>>;

inline const auto fail = std::nullopt;

template <typename T>
inline const auto fail_as = static_cast<T>(std::nullopt);

template <
	typename TValue,
	forward_range TSource
	>
inline MaybeMatch<TValue, TSource> makeMaybeMatch(
	TValue value,
	View<TSource> next
) {
	return Match<TValue, TSource> {
		value, next
	};
}

template <
	typename TValue,
	forward_range TSource
	>
inline std::optional<TValue> unwrap(
	MaybeMatch<TValue, TSource> match
) {
	return match ?
		std::make_optional(match->value)
		: fail;
}

template <
	typename TValue,
	forward_range TSource
	>
std::ostream& operator<<(
	std::ostream& os, const Match<TValue, TSource> match
) {
	os << "{" << match.value << "}";
	return os;
}

template <
	typename TValue,
	forward_range TSource
	>
std::ostream& operator<<(
	std::ostream& os, const MaybeMatch<TValue, TSource> mmatch
) {
	if (mmatch) {
		os << "{?" << mmatch->value << "?}";
	}
	else {
		os << "{-fail-}";
	}
	return os;
}

}
