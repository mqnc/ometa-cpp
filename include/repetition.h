#pragma once

#include <deque>

#include "parser.h"

namespace ometa {

template <typename T>
using RepetitionValue = std::deque<T>;
template <typename T>
concept RepetitionValueType = std::is_same_v<
	T, RepetitionValue<typename T::value_type>
	>;

template <typename T>
auto repetition(T child, size_t min, size_t max) {

	auto parseFn = [child, min, max]<forward_range TSource>
		(
			View<TSource> src,
			const auto& ctx
		) {

			using return_element_type = decltype(child.parseOn(src, ctx)->value);
			RepetitionValue<return_element_type> matches {};

			auto next = src;

			for (size_t i = 0; i < max; i++) {
				auto result = child.parseOn(next, ctx);

				if (result.has_value()) {
					matches.push_back(result->value);
					next = result->next;
				}
				else if (i < min) {
					return fail_as<decltype(makeMaybeMatch(matches, next))>;
				}
				else {
					break;
				}
			}

			return makeMaybeMatch(matches, next);

		};

	return Parser(parseFn);
}

template <typename F>
auto operator-(Parser<F> parser) {
	return repetition(parser, 0, 1);
}
template <typename F>
auto operator*(Parser<F> parser) {
	return repetition(parser, 0, (size_t) -1);
}
template <typename F>
auto operator+(Parser<F> parser) {
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