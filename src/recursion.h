#pragma once

#include <memory>
#include "parser.h"
#include "empty.h"

namespace ometa {



template <typename TSource, typename TValue, typename TContext = Empty>
auto declare() {
	return std::make_shared<
		Parser<std::function<MaybeMatch<TValue, TSource>(SourceView<TSource>, TContext)>>
	>(
		[](SourceView<TSource> src, TContext ctx) -> MaybeMatch<TValue, TSource> {
			throw std::runtime_error("forward-declared parser not initialized");
		}
	);
}

auto ptr = []<typename F>(std::shared_ptr<Parser<F>> target) {
	auto parseFn = [target]<forward_range TSource>
		(
			SourceView<TSource> src,
			auto ctx
		) {
			return target->parseOn(src, ctx);
		};
	return Parser(parseFn);
};

}
