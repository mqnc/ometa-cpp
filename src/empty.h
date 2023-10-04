#pragma once

#include <ostream>

namespace ometa {

struct Empty {};

constexpr bool operator==(const Empty, const Empty) {
	return true;
}

constexpr bool operator==(const auto, const Empty) {
	return false;
}

Empty empty;

std::ostream& operator<<(std::ostream& os, Empty)
{
	os << "(empty)";
	return os;
}

}
