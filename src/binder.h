#pragma once

#include "parser.h"
#include "stringliteral.h"

template <typename T, StringLiteral Tag>
auto makeBinder(T child, Binding<Tag>) {

	auto parseFn = [child]<forward_range TSource>
		(
			SourceView<TSource> src,
			auto ctx
		) {

			auto result = child.parseOn(src, ctx);
			return (result) ?
				std::make_optional(tag<Tag>(std::move(*result)))
				: fail;
		};

	return Parser(parseFn);
}

template <typename F, StringLiteral Tag>
auto operator-(Parser<F> parser, Binding<Tag> bind) {
	return makeBinder(parser, bind);
}

#define AS(NAME) -binding<#NAME>
