#pragma once

#include "parser.h"

namespace ometa{

// wrapper around another parser with access to said parser

template <typename Tag, typename F>
class ParserWrapper: public Parser<Tag, F>{
public:
	using Parser<Tag, F>::Parser;
	
	const auto& getChild() const{
		return this->parseFn.child;
	}
};

template <DerivedFromParser T>
class WrapperFn {
public:
	T child;
	WrapperFn(T parser):child{parser}{};
	auto operator()(auto src, auto& ctx) const {
		return child.parseOn(src, ctx);
	}
};

}
