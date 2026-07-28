#pragma once

#include <memory>
#include "parser.h"
#include "empty.h"

namespace ometa{

template <typename F, typename TSetter>
class RecursiveParser : public Parser<F> {
public:
	const TSetter define;
    RecursiveParser(F fn, TSetter setter)
        : Parser<F>{fn}, define{setter}
    {}
};

template <typename TSource, typename TValue, typename TContext = Empty>
auto recursive(){

	auto wrappedChild =
		std::make_shared<std::function<MaybeMatch<TValue, TSource>(View<TSource>, TContext&)>>(
			[](View<TSource> src, TContext& ctx) -> MaybeMatch<TValue, TSource> {
				throw std::runtime_error("recursive parser not defined");
			}
		);

	auto parseFn = [wrappedChild](View<TSource> src, TContext& ctx){
		return (*wrappedChild)(src, ctx);
	};

	auto define = [pWrapped = wrappedChild.get()](DerivedFromParser auto child){
		*pWrapped = [child](View<TSource> src, TContext& ctx){
			return child.parseOn(src, ctx);
		};
	};

	return RecursiveParser(parseFn, define);
}

}