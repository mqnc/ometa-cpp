#pragma once

#include "parser.h"

template <typename TSource, typename TValue, typename TContext = decltype(Context {})>
auto makeDummy() {
	return Parser<std::function<MaybeMatch<TSource, TValue>(SourceView<TSource>, TContext)>> {
		[](SourceView<TSource> src, TContext ctx) -> MaybeMatch<TSource, TValue> {
			throw std::runtime_error("forward-declared parser not initialized");
		}
	};
}

template <typename F>
auto makeReference(const Parser<F>& target) {

	auto parseFn = [&target]<forward_range TSource>
		(
			SourceView<TSource> src,
			auto ctx
		) {
			return target.parse(src, ctx);
		};

	return Parser(parseFn);
}

#define REF(x) makeReference(x)
