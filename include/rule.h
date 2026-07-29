#pragma once

#include "parser.h"
#include "debug.h"

namespace ometa{

template <typename F>
class RuleWrapper: public Parser<F>{
public:
	using Parser<F>::Parser;
	
	const auto& getBody() const{
		return this->parseFn.body;
	}
};

template <Tag tag, DerivedFromParser T>
class Rule {
public:
	T body;
	Rule(T parser):body{parser}{};
	auto operator()(auto src, auto& ctx) const {
		#ifdef DEBUG_PRINTS
			log(tag.value, LogEvent::enter, src);
			auto result = body.parseOn(src, ctx);
			auto evt = result ? LogEvent::accept : LogEvent::reject;
			log(tag.value, evt, src, result->next);
			return result;
		#else
			return body.parseOn(src, ctx);
		#endif
	}
};

template <Tag tag, DerivedFromParser T>
inline auto rule(T body) {
	return RuleWrapper<Rule<tag, T>>(Rule<tag, T>(body));
}

}
