#pragma once

#include "parser.h"

template <forward_range TSource, typename TValue>
struct Capture {
	SourceView<TSource> match;
	TValue value;
};

template <typename T>
auto makeCapture(T child) {

	auto parseFn = [child]<forward_range TSource>
		(
			SourceView<TSource> src,
			auto ctx
		) {

			auto result = child.parseOn(src, ctx);

			return result.has_value() ?
				match(
					Capture<TSource, decltype(result->value)> {
						SourceView<TSource>(
							src.begin(),
							result->next.begin()
							),
						result->value
					},
					result->next
					)
				: fail;
		};

	return Parser(parseFn);
}

#define CAP(x) makeCapture(x)
