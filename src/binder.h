#include "parser.h"
#include "stringliteral.h"

template<typename T, StringLiteral Tag>
auto makeBinder(T child, Binding<Tag>) {

	auto parseFn = []<forward_range TSource>(
		SourceView<TSource> src,
		auto children,
		auto ctx
		) {
        
		auto result = std::get<0>(children).parse(src, ctx);
		return (result) ? std::make_optional(
			std::move(result)->template toTagged<Tag>()
		) : fail;
	};

	return Parser(parseFn, std::make_tuple(child));
}

template <typename F, typename TChildren, StringLiteral Tag>
auto operator - (Parser<F, TChildren> parser, Binding<Tag> bind) {
	return makeBinder(parser, bind);
}

#define AS(NAME) - binding<#NAME>