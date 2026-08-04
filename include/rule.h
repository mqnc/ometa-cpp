#pragma once

#include "wrapper.h"
#include "log.h"

namespace ometa{

template <typename Tag, DerivedFromParser T>
inline auto rule(T body);

template <typename Tag, typename F>
class RuleWrapper: public ParserWrapper<Tag, F>{
public:
	using ParserWrapper<Tag, F>::ParserWrapper;

	auto withChild(auto child) const {
		return rule<Tag>(child);
	}
};

template <typename Tag, DerivedFromParser T>
inline auto rule(T body) {
	return RuleWrapper<Tag, WrapperFn<T>>(WrapperFn<T>(body));
}

}
