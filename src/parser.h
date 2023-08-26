#pragma once

#include <tuple>
#include <string>

#include "empty.h"
#include "sourceview.h"
#include "match.h"
#include "context.h"

template <typename F>
class Parser {
protected:
	F parseFn;
public:

	Parser(F parseFn): parseFn {parseFn} {}

	template <typename TCtx = decltype(Context {})>
	auto parse(const auto& src, TCtx ctx = Context {}) const {
		return parseFn(SourceView(src), ctx);
	}

	template <forward_range TSource>
	auto parse(SourceView<TSource> src, auto ctx) const {
		return parseFn(src, ctx);
	}

	// template <typename TSource>
	// using TValue = decltype(std::declval<Parser<F, TChildren>>().parse(std::declval<TSource>()));

};
