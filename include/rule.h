
#include "parser.h"
#include "debug.h"

namespace ometa{

#ifdef DEBUG_PRINTS

template <Tag tag, DerivedFromParser T>
class Rule {
	T body;
public:
	Rule(T parser):body{parser}{};
	auto operator()(auto src, auto& ctx) const {
		log(tag.value, LogEvent::enter, src);
		auto result = body.parseOn(src, ctx);
		auto evt = result ? LogEvent::accept : LogEvent::reject;
		log(tag.value, evt, src, result->next);
		return result;
	}
};

template <Tag tag, DerivedFromParser T>
auto rule(T body) {
	return Parser(Rule<tag, T>(body));
}

#else

template <Tag tag, DerivedFromParser T>
auto rule(T body) {
	return body;
}

#endif

}
