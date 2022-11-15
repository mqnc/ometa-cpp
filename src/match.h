#pragma once

#include "sourceview.h"

template<forward_range TSource, typename TValue>
struct Match {
	SourceView<TSource> next;
	TValue value;
};

template<forward_range TSource, typename TValue>
using MaybeMatch = std::optional<Match<TSource, TValue>>;

inline const auto fail = std::nullopt;

template<typename T>
inline const auto fail_as = static_cast<T>(std::nullopt);

template<forward_range TSource, typename TValue>
inline MaybeMatch<TSource, TValue> match(
	SourceView<TSource> next,
	TValue value
) {
	return Match<TSource, TValue>{next, value};
}
