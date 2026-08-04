#pragma once

#include <ostream>

namespace ometa {

struct Empty {};

template <typename T>
constexpr bool isEmpty(const T&) {
	return std::same_as<std::remove_cvref_t<T>, Empty>;
}

inline Empty empty;

std::ostream& operator<<(std::ostream& os, Empty)
{
	os << "(empty)";
	return os;
}

}
