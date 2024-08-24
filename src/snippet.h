#pragma once

#include "empty.h"
#include <variant>
#include <memory>
#include <array>

namespace ometa {

template <typename TData>
class Snippet {

	using TChildren = std::array<std::shared_ptr<struct Snippet<TData>>, 2>;

	std::variant<
		Empty,
		View<TData>,
		TChildren
	> value;

	Snippet(TChildren children): value(children) {}

public:
	Snippet(): value(empty) {}
	Snippet(View<TData> view): value(view) {}

	Snippet<TData> operator+(Snippet<TData> other) {
		return typename Snippet<TData>::TChildren({
			std::make_shared<Snippet<TData>>(*this),
			std::make_shared<Snippet<TData>>(other)
		});
	}

	template <typename TStream>
	void collect(TStream& stream) {
		std::visit( [&stream](auto&& val) {
				if constexpr (std::is_same_v<std::decay_t<decltype(val)>, Empty>) {
					(void) val;
				} else if constexpr (std::is_same_v<std::decay_t<decltype(val)>, View<TData>>) {
					val.copyInto(stream);
				} else {
					val[0]->collect(stream);
					val[1]->collect(stream);
				}
			}, value);
	}

	template <typename TStream>
	TStream collect() {
		TStream stream;
		collect(stream);
		return stream;
	}
};

}
