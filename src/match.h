#pragma once

#include <ostream>

#include "empty.h"
#include "sourceview.h"
#include "stringliteral.h"

template <forward_range TSource, typename TValue, StringLiteral Tag = "">
struct Match {
	SourceView<TSource> capture;
	SourceView<TSource> next;
	TValue value;

	template <StringLiteral NewTag>
	auto toTagged() {
		return Match<TSource, TValue, NewTag> {
			capture,
			next,
			value
		};
	}
};

template <forward_range TSource, typename TValue, StringLiteral Tag = "">
using MaybeMatch = std::optional<Match<TSource, TValue, Tag>>;

inline const auto fail = std::nullopt;

template <typename T>
inline const auto fail_as = static_cast<T>(std::nullopt);

template <forward_range TSource, typename TValue, StringLiteral Tag = "">
inline MaybeMatch<TSource, TValue, Tag> match(
	SourceView<TSource> src,
	SourceView<TSource> next,
	TValue value
) {
	return Match<TSource, TValue, Tag> {
		SourceView<TSource>(src.begin(), next.begin()),
		next,
		value
	};
}

template <forward_range TSource, typename TValue, StringLiteral Tag>
std::ostream& operator<<(std::ostream& os, const Match<TSource, TValue, Tag> match)
{
	os << "\"" << match.capture << "\"";
	if constexpr (Tag != "") { os << ":" << Tag; }
	os << " -> " << match.value;
	return os;
}

template <forward_range TSource, typename TValue, StringLiteral Tag>
std::ostream& operator<<(std::ostream& os, const MaybeMatch<TSource, TValue, Tag> mmatch)
{
	if (mmatch) {
		os << "*" << mmatch.value;
	}
	else {
		os << "fail";
	}
	return os;
}
