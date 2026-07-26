
#include "parser.h"
#include "debug.h"

namespace ometa{

#ifdef DEBUG_PRINTS

template <Tag tag, DerivedFromParser P>
class Rule {
	P body;
public:
	Rule(P parser):body{parser}{};
	auto operator()(auto src, auto& ctx) const {
		log(tag.value, LogEvent::enter, src);
		auto result = body.parseOn(src, ctx);
		auto evt = result ? LogEvent::accept : LogEvent::reject;
		log(tag.value, evt, src, result->next);
		return result;
	}
};

template <Tag tag, DerivedFromParser P>
auto rule(P body) {
	return Parser(Rule<tag, P>(body));
}

#else

template <Tag tag, DerivedFromParser P>
auto rule(P body) {
	return body;
}

#endif

}
