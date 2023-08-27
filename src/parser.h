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

	template <typename TCtx = decltype(Context {})>
	auto parse(const auto& src, TCtx ctx = Context {}) const {
		return parseFn(SourceView(src), ctx);
	}

	template <forward_range TSource>
	auto parse(SourceView<TSource> src, auto ctx) const {
		return parseFn(src, ctx);
	}

	template <typename F2>
	void operator=(const Parser<F2>& target) {
		parseFn = target.parseFn;
	}

	// template <typename TSource>
	// using TValue = decltype(std::declval<Parser<F, TChildren>>().parse(std::declval<TSource>()));

};
