#pragma once

#include <tuple>
#include <string>
#include <functional>

#include "empty.h"
#include "sourceview.h"
#include "match.h"
#include "tag.h"

template <typename F>
class Parser {
public:
	F parseFn;

	Parser(F parseFn): parseFn {parseFn} {}

	// to be called from the outside to start the parsing process
	template <typename TCtx = Empty>
	auto parse(const auto& src, TCtx ctx = empty) const {
		auto result = parseFn(SourceView(src), ctx);
		return unwrap(result);
	}

	// to be called internally by parent parsers
	template <forward_range TSource>
	auto parseOn(SourceView<TSource> src, auto ctx) const {
		return parseFn(src, ctx);
	}

	template <typename F2>
	void operator=(const Parser<F2>& target) {
		parseFn = target.parseFn;
	}

	template <Tag tag>
	auto as() {

		auto parseFn = [this]<forward_range TSource>
			(
				SourceView<TSource> src,
				auto ctx
			) {
				auto result = this->parseOn(src, ctx);
				return (result) ?
					makeMaybeMatch(
						makeTagged<tag>(result->value),
						result->next
					)
					: fail;
			};

		return Parser<decltype(parseFn)>(parseFn);
	}

	// template <typename TSource>
	// using TValue = decltype(std::declval<Parser<F, TChildren>>().parse(std::declval<TSource>()));

};
