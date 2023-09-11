#pragma once

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <utility>
#include <string_view>

template <size_t N>
struct Tag {
	constexpr Tag(const char (&str)[N]) {
		std::copy_n(str, N, value);
	}
	char value[N];

	template <size_t N2>
	constexpr bool operator==(const Tag<N2>& other) const {
		return std::string_view(this->value) == other.value;
	}

	constexpr bool operator==(const char* other) const {
		return std::string_view(this->value) == other;
	}
};

template <Tag tag, typename TValue>
struct Tagged {
	static constexpr auto getTag() { return tag; }
	TValue value;
};

template <Tag tag, typename TValue>
Tagged<tag, TValue> makeTagged(const TValue& value) {
	return {value};
}

template <typename T>
concept IsTagged = std::is_same_v<
	T,
	Tagged<T::getTag(), decltype(std::declval<T>().value)>
	>;
