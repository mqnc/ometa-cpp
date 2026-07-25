#pragma once

#include <memory>
#include "parser.h"
#include "empty.h"

namespace ometa {

template <typename F>
class MutableParser: public Parser<F>{
public:
	using Parser<F>::Parser;
	
	template<DerivedFromParser P>
	MutableParser<F>& operator=(const P& other) {
		this->parseFn = other.getParseFn();
		return *this;
	}
};

template <typename TSource, typename TValue, typename TContext = Empty>
auto declare() {
	return std::make_shared<
		MutableParser<std::function<MaybeMatch<TValue, TSource>(View<TSource>, TContext&)>>
	>(
		[](View<TSource> src, TContext& ctx) -> MaybeMatch<TValue, TSource> {
			throw std::runtime_error("forward-declared parser not initialized");
		}
	);
}

template<DerivedFromParser P>
auto ptr(std::shared_ptr<P> target) {
	auto parseFn = [target]<forward_range TSource> (
		View<TSource> src,
		auto& ctx
	) {
		return target->parseOn(src, ctx);
	};

	return Parser(parseFn);
}

}
