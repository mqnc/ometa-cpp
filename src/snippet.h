#pragma once

#include "tree.h"
#include <variant>
#include <memory>

namespace ometa {

template <typename TData>
struct Snippet {
	std::variant<
		View<TData>,
		ValueTree<
			std::shared_ptr<struct Snippet<TData>>,
			std::shared_ptr<struct Snippet<TData>>
			>
		> value;

	template <typename TStream>
	void collect(TStream& stream) {
		std::visit( [&stream](auto&& val) {
				if constexpr (std::is_same_v<std::decay_t<decltype(val)>, View<TData>>) {
					val.copyInto(stream);
				} else {
					val.left->collect(stream);
					val.right->collect(stream);
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

template <typename TData>
Snippet<TData> operator+(Snippet<TData> lhs, Snippet<TData> rhs) {
	return {ValueTree<
		std::shared_ptr<Snippet<TData>>,
		std::shared_ptr<Snippet<TData>>
		>(
		std::make_shared<Snippet<TData>>(std::move(lhs)),
		std::make_shared<Snippet<TData>>(std::move(rhs))
	)};
}

}
