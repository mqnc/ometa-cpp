#pragma once

#include <deque>

#include "parser.h"

template<typename T>
auto makeRepetition(T child, size_t min, size_t max) {

	auto parseFn = [min, max]<forward_range TSource>(
		SourceView<TSource> src,
		auto children,
		auto ctx
		) {

		using return_element_type = decltype(std::get<0>(children).parse(src, ctx)->value);
		std::deque<return_element_type> results{};

		auto next = src;

		for (size_t i = 0; i < max; i++) {
			auto result = std::get<0>(children).parse(next, ctx);

			if (result.has_value()) {
				results.push_back(result->value);
				next = result->next;
			}
			else if (i < min) {
				return fail_as<decltype(match(next, results))>;
			}
			else {
				break;
			}
		}

		return match(next, results);

	};

	return Parser(parseFn, std::make_tuple(child));
}

template <typename F, typename TChildren>
auto operator~ (Parser<F, TChildren> parser) {
	return makeRepetition(parser, 0, 1);
}
template <typename F, typename TChildren>
auto operator* (Parser<F, TChildren> parser) {
	return makeRepetition(parser, 0, (size_t)-1);
}
template <typename F, typename TChildren>
auto operator+ (Parser<F, TChildren> parser) {
	return makeRepetition(parser, 1, (size_t)-1);
}
