
#include "src/ometa.h"

#include <cassert>
#include <iostream>
#include <sstream>

#include <map>

using std::get;

int main() {

	assert(ANY.parse("1"));
	assert(not ANY.parse(""));

	auto abc = "abc"_L;
	auto def = "def"_L;
	auto ghi = "ghi"_L;

	auto lit = abc;
	assert(*lit.parse("abcd") == "abc");
	assert(not lit.parse("abX"));
	assert(not lit.parse(""));

	auto rng = RNG('a', 'm');
	assert(*rng.parse("m") == "m");
	assert(not rng.parse("n"));

	auto seq = abc > def > ghi;

	assert(get<0>(*seq.parse("abcdefghi")) == "abc");
	assert(get<1>(*seq.parse("abcdefghi")) == "def");
	assert(get<2>(*seq.parse("abcdefghi")) == "ghi");

	assert(not seq.parse("abcdefghX"));

	// enforce same semantic value type of all branches
	auto cho1 = abc | def | ghi;
	assert(*cho1.parse("abc") == "abc");
	assert(*cho1.parse("def") == "def");
	assert(*cho1.parse("ghi") == "ghi");
	assert(not cho1.parse("XXX"));

	// create variant of semantic value types of all branches
	auto cho2 = abc || def || ghi;
	assert(cho2.parse("abc")->index() == 0);
	assert(cho2.parse("def")->index() == 1);
	assert(cho2.parse("ghi")->index() == 2);
	assert(not cho2.parse("XXX"));

	auto pla = &abc;
	assert(pla.parse("abc"));
	assert(not pla.parse("XXX"));

	auto nla = !abc;
	assert(not nla.parse("abc"));
	assert(nla.parse("XXX"));

	auto opt = ~abc > def;
	assert(opt.parse("abcdef"));
	assert(opt.parse("def"));
	assert(not opt.parse("XXX"));

	auto zom = *abc > def;
	assert(zom.parse("abcabcdef"));
	assert(get<0>(*zom.parse("abcabcdef")).size() == 2);
	assert(zom.parse("abcdef"));
	assert(zom.parse("def"));
	assert(not zom.parse("XXX"));

	auto oom = +abc > def;
	assert(oom.parse("abcabcdef"));
	assert(oom.parse("abcdef"));
	assert(not oom.parse("def"));
	assert(not oom.parse("XXX"));

	auto cap = CAP(oom);
	assert(cap.parse("abcabcdef---")->match == "abcabcdef");

	auto act = abc >= [](auto matched) {
		assert(matched == "abc");
		return 123;
	};
	assert(*act.parse("abc") == 123);

	auto prd1 = abc <= [](auto matched) {
		assert(*matched == "abc");
		return true;
	};
	assert(*prd1.parse("abc") == "abc");

	auto prd0 = abc <= [](auto matched) {
		assert(not matched);
		return false;
	}; 
	assert(not prd0.parse("XXX"));


	Parser check(
		[](auto src, auto ctx) {
			(void) src;

			if constexpr (not ctx.is_empty()) {
				assert(ctx.GET(t0) == "abc");
				assert(get<1>(ctx.GET(ts)[1]) == "abc");
			}

			return makeMaybeMatch(empty, src);
		}
	);

	auto list = abc AS(t0) > *("+"_L > abc) AS(ts) > check;
	list.parse("abc+abc+abc");

	auto expression = makeDummy<std::string_view, std::string>();

	expression = ("atom"_L || "("_L > REF(expression) > ")"_L) >= [](auto matched) {
		std::stringstream ss;
		switch (matched.index()) {
			case 0:
				ss << get<0>(matched);
				break;
			case 1: {
				auto tuple = get<1>(matched);
				ss << get<2>(tuple)
				   << get<1>(tuple)
				   << get<0>(tuple);
				break;
			}
		}
		return ss.str();
	};

	assert(*expression.parse("atom") == "atom");
	assert(*expression.parse("(atom)") == ")atom(");
	assert(*expression.parse("((atom))") == "))atom((");

	std::cout << "done!\n";
	return 0;
}
