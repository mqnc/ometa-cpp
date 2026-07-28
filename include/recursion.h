#pragma once

#include <memory>
#include "parser.h"
#include "empty.h"

namespace ometa{

template <typename TSource, typename TValue, typename TContext = Empty>
class MutableParser: public Parser<
	std::function<MaybeMatch<TValue, TSource>(View<TSource>, TContext&)>
>{
public:

	MutableParser():Parser<std::function<MaybeMatch<TValue, TSource>(View<TSource>, TContext&)>>{
		[](View<TSource> src, TContext& ctx) -> MaybeMatch<TValue, TSource> {
			throw std::runtime_error("mutable parser not defined");
		}
	}{}

	template<DerivedFromParser P>
	void setChild(const P& child){
		this->parseFn = [child] (
			View<TSource> src,
			auto& ctx
		) {
			return child.parseOn(src, ctx);
		};
	}
};

template <typename F, typename SC>
class SharedParser:public Parser<F>{
public:
	SC setChild;
	SharedParser(F parseFn, SC setChild):Parser<F>{parseFn}, setChild{setChild}{}
};

template <typename TSource, typename TValue, typename TContext = Empty>
auto declareSharedMutableParser() {

	auto sharedParser = std::make_shared<MutableParser<TSource, TValue, TContext>>();

	auto parseFn = [sharedParser] (
		View<TSource> src,
		auto& ctx
	) {
		return sharedParser->parseOn(src, ctx);
	};

	auto setChild = [sharedParser] (auto child){
		sharedParser->setChild(child);
	};

	return SharedParser<decltype(parseFn), decltype(setChild)>(parseFn, setChild);
}

}
