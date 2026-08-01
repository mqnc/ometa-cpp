#pragma once

#include "parser.h"
#include "log.h"

namespace ometa{

template <typename Tag, typename F>
class RuleWrapper: public Parser<Tag, F>{
public:
	using Parser<Tag, F>::Parser;
	
	// some sub parsers require us to unpack the child and wrap it
	// in another parent and wrap that with this rule again
	const auto& getBody() const{
		return this->parseFn.body;
	}
};

template <DerivedFromParser T>
class Rule { // can't use lambda because we need to extract the body from the capture
public:
	T body;
	Rule(T parser):body{parser}{};
	auto operator()(auto src, auto& ctx) const {
		return body.parseOn(src, ctx);
	}
};

template <typename Tag, DerivedFromParser T>
inline auto rule(T body) {
	return RuleWrapper<Tag, Rule<T>>(Rule<T>(body));
}

}
