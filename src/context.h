#pragma once

#include <type_traits>

#include "empty.h"

template<typename TOuter = Empty, typename Tag = Empty, typename TValue = Empty>
class Context {
	const TOuter& outer;
	const TValue value;

	Context(
		const TOuter& outer,
		const TValue& value
	) :outer{ outer }, value{ value } {}

public:

	Context() :outer{ Empty{} }, value{ Empty{} } {}

	template<typename NewTag, typename TNewValue>
	auto add(TNewValue nv) {
		return Context<decltype(*this), NewTag, TNewValue>(
			*this, nv);
	}

	template<typename GetTag>
	auto get() const {
		if constexpr (std::is_same_v<GetTag, Tag>) {
			return value;
		}
		else {
			return outer.template get<GetTag>();
		}
	}

	template<typename, typename, typename>
	friend class Context;
};

template <typename>
struct is_context :public std::false_type {};
template <typename T, typename U, typename V>
struct is_context<Context<T, U, V>> :public std::true_type {};
template<typename T>
concept context_type = is_context<T>::value;

