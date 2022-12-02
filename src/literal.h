#pragma once

#include <string>
#include <string_view>
#include <algorithm>

#include "parser.h"

using namespace std::string_literals;

auto makeLiteral(auto compare) {

	auto parseFn = [compare]<forward_range TSource>(
		SourceView<TSource> src,
		auto children,
		auto ctx
		) {
		(void)children;
		(void)ctx;

		auto equalUntil = std::ranges::mismatch(src, compare);

		return equalUntil.in2 == compare.end() ? [&] {
			auto next = SourceView<TSource>(equalUntil.in1, src.end());
			auto matched = SourceView<TSource>(src.begin(), equalUntil.in1);
			return match(src, next, matched);
		}() : fail;
	};

	return Parser(parseFn);
}

auto makeLiteral(const char* compare){
	return makeLiteral(std::string_view(compare));
}

auto operator""_L(const char* compare, size_t size) {
	return makeLiteral<std::string>(std::string(compare, size));
}
