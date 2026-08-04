#pragma once

#include "parser.h"

namespace ometa{

template<typename>
inline constexpr bool always_false_v = false;

// wrapper around another parser with access to said parser
template <typename Tag, typename F>
class ParserWrapper: public Parser<Tag, F>{
public:
	using Parser<Tag, F>::Parser;
	
	const auto& getChild() const{
		return this->parseFn.child; // this->parseFn must have a public child
	}

	auto withChild(auto child) const {
		static_assert(always_false_v<Tag>, "withChild() must be overwritten by deriving wrapper classes");
		return false;
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

template<typename T>
concept DerivedFromWrapper = DerivedFromParser<T>
	&& requires {typename T::parse_fn_type;}
	&& std::derived_from<T, ParserWrapper<typename T::TTag, typename T::parse_fn_type>>;

template <DerivedFromParser T, DerivedFromWrapper W>
auto operator>=(T parser, W wrapper) {
	return wrapper.withChild(parser >= wrapper.getChild());
}

}
