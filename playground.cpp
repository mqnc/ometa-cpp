
#pragma once

#include <chrono>
#include <ostream>
#include <string>

class Timer {
public:
	std::chrono::time_point<std::chrono::high_resolution_clock> t0;

	Timer(): t0 {std::chrono::high_resolution_clock::now()} {}

	std::chrono::duration<double> elapsed() const {
		auto t = std::chrono::high_resolution_clock::now();
		return t - t0;
	}

	std::string elapsedFormatted() const {
		auto delta = elapsed();
		if (delta < std::chrono::milliseconds {1}) {
			return std::to_string(std::chrono::duration<double, std::micro>(delta).count()) + " µs";
		}
		if (delta < std::chrono::seconds {1}) {
			return std::to_string(std::chrono::duration<double, std::milli>(delta).count()) + " ms";
		}
		if (delta < std::chrono::minutes {1}) {
			return std::to_string(delta.count()) + " s";
		}
		if (delta < std::chrono::hours {1}) {
			return std::to_string(std::chrono::duration<double, std::ratio<60>>(delta).count()) + " mins";
		}
		return std::to_string(std::chrono::duration<double, std::ratio<3600>>(delta).count()) + " h";
	}

	double s() const {
		return elapsed().count();
	}
};

std::ostream& operator<<(std::ostream& os, const Timer& t) {
	os << t.elapsedFormatted();
	return os;
}

#include <iostream>
int main() {
	Timer t;
	std::cout << t << "\n";
}
