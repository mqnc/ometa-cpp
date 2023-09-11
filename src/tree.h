#pragma once

#include <tuple>

#include "empty.h"
#include "tag.h"

template <class T>
constexpr bool always_false = false;

template <typename T1, typename T2>
struct ValueTree {
	T1 left;
	T2 right;

	ValueTree(T1 left, T2 right):
		left {left}, right {right} {}

	template <size_t i>
	auto pick();

	template <Tag tag>
	auto pick();

};

auto join(auto value) {
	return value;
}

auto join(auto value, auto... rest) {
	return ValueTree {value, join(rest...)};
}

template <typename>
struct NumLeaves: std::integral_constant<size_t, 1> {};

template <typename T1, typename T2>
struct NumLeaves<ValueTree<T1, T2>>
	: std::integral_constant<size_t, NumLeaves<T1>::value + NumLeaves<T2>::value> {};

// named pick to avoid ADL clash with std::get
template <size_t i, typename TValue>
inline auto pick(const TValue& value) {
	static_assert(i == 0);
	return value;
}

template <size_t i, Tag tag, typename TValue>
inline auto pick(const Tagged<tag, TValue>& tagged) {
	static_assert(i == 0);
	return tagged.value;
}

template <size_t i, typename T1, typename T2>
inline auto pick(const ValueTree<T1, T2>& tree) {
	if constexpr (i < NumLeaves<T1>::value) {
		return pick<i>(tree.left);
	}
	else if constexpr (i < NumLeaves<T1>::value + NumLeaves<T2>::value) {
		return pick<i - NumLeaves<T1>::value>(tree.right);
	}
	else {
		static_assert(always_false<T1>, "index out of range");
	}
}

template <typename, Tag>
struct Contains: std::false_type {};

template <typename TValue, Tag tag>
struct Contains<Tagged<tag, TValue>, tag>: std::true_type {};

template <typename T1, typename T2, Tag tag>
struct Contains<ValueTree<T1, T2>, tag>
	: std::disjunction<Contains<T1, tag>, Contains<T2, tag>> {};

template <Tag tag, typename TValue>
inline auto pick(const Tagged<tag, TValue>& tagged) {
	return tagged.value;
}

template <Tag tag, typename T1, typename T2>
inline auto pick(const ValueTree<T1, T2>& tree) {
	if constexpr (Contains<T1, tag>::value) {
		return pick<tag>(tree.left);
	}
	else if constexpr (Contains<T2, tag>::value) {
		return pick<tag>(tree.right);
	}
	else {
		static_assert(always_false<T1>, "tree must contain value with requested tag");
	}
}

template <typename T1, typename T2>
template <size_t i>
auto ValueTree<T1, T2>::pick(){
	return ::pick<i>(*this);
}

template <typename T1, typename T2>
template <Tag tag>
auto ValueTree<T1, T2>::pick(){
	return ::pick<tag>(*this);
}

template <typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const ValueTree<T1, T2>& tree) {
	os << tree.left << "\n";
	os << tree.right;
	return os;
}
