#pragma once

#include <tuple>
#include <string>
#include <functional>

#include "empty.h"
#include "sourceview.h"
#include "match.h"
#include "context.h"

template <typename F>
class Parser {
public:
	F parseFn;

	Parser(F parseFn): parseFn {parseFn} {}

	// to be called from the outside to start the parsing process
	template <typename TCtx = decltype(Context {})>
	auto parse(const auto& src, TCtx ctx = Context {}) const {
		auto result = parseFn(SourceView(src), ctx);
		// unwrap match, we don't need the .next member
		return result ?
			std::make_optional(result->value)
			: fail;
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

	// template <typename TSource>
	// using TValue = decltype(std::declval<Parser<F, TChildren>>().parse(std::declval<TSource>()));

};
