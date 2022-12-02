#pragma once

#include <tuple>
#include <string>

#include "empty.h"
#include "sourceview.h"
#include "match.h"
#include "context.h"

template <typename F, typename TChildren = std::tuple<>>
class Parser {
protected:
	F parseFn;
public:

	TChildren children;

	Parser(F parseFn, TChildren children = std::tuple<>{}) :
		parseFn{ parseFn }, children{ children } {}

	template<typename TCtx = decltype(Context{})>
	auto parse(const auto& src, TCtx ctx = Context{}) {
		return parseFn(SourceView(src), children, ctx);
	}

	template<forward_range TSource>
	auto parse(SourceView<TSource> src, auto ctx) {
		return parseFn(src, children, ctx);
	}

	// template <typename TSource>
	// using TValue = decltype(std::declval<Parser<F, TChildren>>().parse(std::declval<TSource>()));

};
