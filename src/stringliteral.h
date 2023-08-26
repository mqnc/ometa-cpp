
#pragma once

#include <cstdlib>
#include <cstring>
#include <ostream>


template <size_t N>
struct StringLiteral {
	constexpr StringLiteral(const char (&str)[N]) {
		std::copy_n(str, N, value);
	}
	char value[N];

	template <size_t N2>
	constexpr bool operator==(const StringLiteral<N2>& other) const {
		return std::strcmp(this->value, other.value) == 0;
	}

	constexpr bool operator==(const char* other) const {
		return std::strcmp(this->value, other) == 0;
	}
};

// Binding allows to pass template arguments as regular arguments
template <StringLiteral Tag>
class Binding {};
template <StringLiteral Tag>
const Binding<Tag> binding {};

template <size_t N>
std::ostream& operator<<(std::ostream& os, const StringLiteral<N> s)
{
	os << s.value;
	return os;
}
