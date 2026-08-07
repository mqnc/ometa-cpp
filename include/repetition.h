#pragma once

#include <deque>

#include "parser.h"
#include "ignore.h"

namespace ometa {

DECL_DEBUG_TAG(REPETITION, "(repetition)", false);

template <typename T>
using RepetitionValue = std::deque<T>;
template <typename T>
concept RepetitionValueType = std::is_same_v<
	T, RepetitionValue<typename T::value_type>
	>;

template <DerivedFromParser T>
auto repetition(T child, size_t min, size_t max) {

	auto parseFn = [child, min, max]<forward_range TSource>
		(
			View<TSource> src,
			auto& ctx
		) {

			using ResultElementType = decltype(child.parseOn(src, ctx)->value);
			constexpr bool ignoreValues =
				std::same_as<std::remove_cvref_t<ResultElementType>, Ignore>;

			using ResultType = std::conditional_t<
				ignoreValues,
				Ignore,
				RepetitionValue<ResultElementType>
			>;

			ResultType result{};

			auto next = src;
			bool success = true;
			for (size_t i = 0; i < max; i++) {
				auto childResult = child.parseOn(next, ctx);
				if (childResult.has_value()) {
					if constexpr(!ignoreValues){
						result.push_back(childResult->value);
					}
					next = childResult->next;
				}
				else {
					success = i >= min;
					break;
				}
			}
			return success? makeMaybeMatch(result, next) : fail;
		};

	return parser<REPETITION>(parseFn);
}

template <DerivedFromParser T>
auto operator-(T parser) {
	return repetition(parser, 0, 1);
}
template <DerivedFromParser T>
auto operator*(T parser) {
	return repetition(parser, 0, (size_t) -1);
}
template <DerivedFromParser T>
auto operator+(T parser) {
	return repetition(parser, 1, (size_t) -1);
}

template <class T>
std::ostream& operator<<(std::ostream& os, std::deque<T> const& vals) {
	os << "[";
	std::string sep = "";
	for (auto val: vals) {
		os << sep << val;
		sep = ", ";
	}
	os << "]";
	return os;
}

}
