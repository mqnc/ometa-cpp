
#include <string>
#include <fstream>
#include <ranges>
#include "tree.h"
#include "repetition.h"

namespace ometa {

inline std::string readFile(const std::string& filename) {
	std::ifstream ifs(filename);
	if (!ifs) {
		throw std::runtime_error(filename + " could not be read");
	}
	return std::string(
		(std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));
}

inline void writeFile(const std::string& filename, const std::string& content) {
	std::ofstream ofs(filename);
	if (!ofs) {
		throw std::runtime_error(filename + " could not be written");
	}
	ofs << content;
}

auto constant = [](auto value) {
	return [value](auto) {
		return value;
	};
};

template <size_t i>
auto select = [](auto value) {
	return pick<i>(value);
};

template <typename T>
auto concatImpl = [](T value) {
	if constexpr (TreeType<T>) {
		return concatImpl<typename T::Type1>(value.left)
			+ concatImpl<typename T::Type2>(value.right);
	}
	else if constexpr (RepetitionValueType<T>) {
		typename T::value_type temp{};
		for(auto item:value){
			temp = temp + item;
		}
		return temp;
	}
	else {
		return value;
	}
};

auto concat = []<typename T>(T value) {
	return concatImpl<T>(value);
};

auto insert = [](auto value) {
	return epsilon() >= constant(value);
};

auto lfold = [](auto combine) {
	return [combine](auto value) {

		auto temp = value.template pick<0>();
		auto steps = value.template pick<1>();

		for (auto step: steps) {
			auto op = step.template pick<0>();
			auto next = step.template pick<1>();
			temp = combine(temp, op, next);
		}

		return temp;
	};
};

auto rfold = [](auto combine) {
	return [combine](auto value) {

		auto steps = value.template pick<1>();
		if (steps.size() == 0) {
			return value.template pick<0>();
		}
		auto op = steps.back().template pick<0>();
		auto temp = steps.back().template pick<1>();

		bool first = true;
		for (auto step: steps | std::views::reverse) {
			if (first) { first = false; continue; }
			auto prev = step.template pick<1>();
			temp = combine(prev, op, temp);
			op = step.template pick<0>();
		}
		auto prev = value.template pick<0>();
		temp = combine(prev, op, temp);

		return temp;
	};
};

}
