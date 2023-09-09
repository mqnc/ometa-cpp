#pragma once

#include <ostream>

#include "empty.h"
#include "sourceview.h"
#include "stringliteral.h"

template <
	StringLiteral Tag,
	typename TValue,
	forward_range TSource
	>
struct Match {
	TValue value;
	SourceView<TSource> next;
};

template <
	StringLiteral NewTag,
	StringLiteral Tag,
	typename TValue,
	forward_range TSource
	>
Match<NewTag, TValue, TSource> tag(Match<Tag, TValue, TSource> m) {
	return {m.value, m.next};
}

template <
	StringLiteral Tag,
	typename TValue,
	forward_range TSource
	>
using MaybeMatch = std::optional<Match<Tag, TValue, TSource>>;

inline const auto fail = std::nullopt;

template <typename T>
inline const auto fail_as = static_cast<T>(std::nullopt);

template <
	StringLiteral Tag = "",
	typename TValue,
	forward_range TSource
	>
inline MaybeMatch<Tag, TValue, TSource> makeMaybeMatch(
	TValue value,
	SourceView<TSource> next
) {
	return Match<Tag, TValue, TSource> {
		value, next
	};
}

template <
	StringLiteral Tag = "",
	typename TValue,
	forward_range TSource
	>
inline std::optional<TValue> unwrap(
	MaybeMatch<Tag, TValue, TSource> match
) {
	return match ?
		std::make_optional(match->value)
		: fail;
}

template <
	StringLiteral Tag,
	typename TValue,
	forward_range TSource
	>
std::ostream& operator<<(
	std::ostream& os, const Match<Tag, TValue, TSource> match
) {
	if constexpr (Tag != "") { os << Tag; }
	os << "{" << match.value << "}";
	return os;
}

template <
	StringLiteral Tag,
	typename TValue,
	forward_range TSource
	>
std::ostream& operator<<(
	std::ostream& os, const MaybeMatch<Tag, TValue, TSource> mmatch
) {
	if constexpr (Tag != "") { os << Tag; }
	if (mmatch) {
		os << "{?" << mmatch->value << "?}";
	}
	else {
		os << "-fail-";
	}
	return os;
}
