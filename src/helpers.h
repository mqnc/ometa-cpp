
#include <ranges>

namespace ometa {

template <size_t i>
auto select = [](auto value) {
	return pick<i>(value);
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
		if(steps.size() == 0){
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
