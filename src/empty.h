#pragma once

#include <ostream>

struct Empty {
	constexpr bool operator==(const Empty other) const {
		return true;
	}
};

Empty empty;

std::ostream& operator<<(std::ostream& os, Empty)
{
	os << "(empty)";
	return os;
}
