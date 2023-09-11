#pragma once

#include "parser.h"
#include "empty.h"

template <typename TSource, typename TValue, typename TContext = Empty>
auto makeDummy() {
	return Parser<std::function<MaybeMatch<TValue, TSource>(SourceView<TSource>, TContext)>> {
		[](SourceView<TSource> src, TContext ctx) -> MaybeMatch<TValue, TSource> {
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
			return target.parseOn(src, ctx);
		};

	return Parser(parseFn);
}

#define REF(x) makeReference(x)
