
#include "parser.h"


auto makeAny() {

	auto parseFn = []<forward_range TSource>(
		SourceView<TSource> src,
		auto children,
		auto ctx
		) {
		(void)children;
		(void)ctx;

        return src.begin() != src.end()? match(src.next(), Empty{}) : fail;
	};

	return Parser(parseFn);
}

#define ANY makeAny()