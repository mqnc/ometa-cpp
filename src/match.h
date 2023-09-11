#pragma once

#include <ostream>

#include "empty.h"
#include "sourceview.h"
#include "tag.h"

template <
	Tag tag,
	typename TValue,
	forward_range TSource
	>
struct Match {
	TValue value;
	SourceView<TSource> next;
};

template <
	Tag newTag,
	Tag tag,
	typename TValue,
	forward_range TSource
	>
Match<newTag, TValue, TSource> relabel(Match<tag, TValue, TSource> m) {
	return {m.value, m.next};
}

template <
	Tag tag,
	typename TValue,
	forward_range TSource
	>
using MaybeMatch = std::optional<Match<tag, TValue, TSource>>;

inline const auto fail = std::nullopt;

template <typename T>
inline const auto fail_as = static_cast<T>(std::nullopt);

template <
	Tag tag = "",
	typename TValue,
	forward_range TSource
	>
inline MaybeMatch<tag, TValue, TSource> makeMaybeMatch(
	TValue value,
	SourceView<TSource> next
) {
	return Match<tag, TValue, TSource> {
		value, next
	};
}

template <
	Tag tag = "",
	typename TValue,
	forward_range TSource
	>
inline std::optional<TValue> unwrap(
	MaybeMatch<tag, TValue, TSource> match
) {
	return match ?
		std::make_optional(match->value)
		: fail;
}

template <
	Tag tag,
	typename TValue,
	forward_range TSource
	>
std::ostream& operator<<(
	std::ostream& os, const Match<tag, TValue, TSource> match
) {
	if constexpr (tag != "") { os << tag; }
	os << "{" << match.value << "}";
	return os;
}

template <
	Tag tag,
	typename TValue,
	forward_range TSource
	>
std::ostream& operator<<(
	std::ostream& os, const MaybeMatch<tag, TValue, TSource> mmatch
) {
	if constexpr (tag != "") { os << tag; }
	if (mmatch) {
		os << "{?" << mmatch->value << "?}";
	}
	else {
		os << "-fail-";
	}
	return os;
}
