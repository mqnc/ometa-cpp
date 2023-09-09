#pragma once

#include <type_traits>

#include "empty.h"
#include "stringliteral.h"

#include <iostream>

template <typename TRest = Empty, StringLiteral Tag = "", typename TValue = Empty>
class Context {
	const TRest rest;
	const TValue value;

	Context(
		const TRest rest,
		const TValue value
		): rest {rest}, value {value} {}

public:

	Context(): rest {empty}, value {empty} {}

	template <StringLiteral NewTag, typename TNewValue>
	auto add(TNewValue nv) {
		return Context<std::remove_reference_t<decltype(*this)>, NewTag, TNewValue>(*this, nv);
	}

	template <StringLiteral GetTag>
	auto get(Binding<GetTag> bind) const {
		if constexpr (GetTag == Tag) {
			return value;
		}
		else if constexpr (not std::is_same_v<TRest, Empty>) {
			return rest.template get(bind);
		}
		else {
			return empty;
		}
	}

	void print() const {
		std::cout << "\"" << Tag << "\"";
		if constexpr (not std::is_same_v<TValue, Empty>) {
			std::cout << "=" << value;
		}
		std::cout << "  ";
		if constexpr (not std::is_same_v<TRest, Empty>) {
			rest.print();
		}
		else {
			std::cout << "\n";
		}
	}

	constexpr bool is_empty() const {
		return std::is_same_v<TValue, Empty> and std::is_same_v<TRest, Empty>;
	}

	template <typename, StringLiteral, typename>
	friend class Context;
};

template <typename>
struct is_context: public std::false_type {};
template <typename T, StringLiteral U, typename V>
struct is_context<Context<T, U, V>>: public std::true_type {};
template <typename T>
concept context_type = is_context<T>::value;

#ifndef GET
	#define GET(NAME) get(binding<#NAME>)
#endif
