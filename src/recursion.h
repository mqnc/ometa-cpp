#pragma once

#include "parser.h"
#include "empty.h"

namespace ometa {

template <typename TSource, typename TValue, typename TContext = Empty>
auto dummy() {
	return Parser<std::function<MaybeMatch<TValue, TSource>(SourceView<TSource>, TContext)>> {
		[](SourceView<TSource> src, TContext ctx) -> MaybeMatch<TValue, TSource> {
			throw std::runtime_error("forward-declared parser not initialized");
		}
	};
}

// using a lambda instead of a function to avoid ADL clash with std::ref
auto ref = []<typename F>(const Parser<F>& target) {
	auto parseFn = [&target]<forward_range TSource>
		(
			SourceView<TSource> src,
			auto ctx
		) {
			return target.parseOn(src, ctx);
		};
	return Parser(parseFn);
};

}
