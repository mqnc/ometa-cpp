
#include "src/ometa.h"

#include <cassert>
#include <iostream>
#include <sstream>

#include <map>

using namespace ometa;

int main() {

	assert(any().parse("1"));
	assert(not any().parse(""));

	auto abc = "abc"_L;
	auto def = "def"_L;
	auto ghi = "ghi"_L;

	auto lit = abc;
	assert(*lit.parse("abcd") == "abc");
	assert(not lit.parse("abX"));
	assert(not lit.parse(""));

	auto rng = range('a', 'm');
	assert(*rng.parse("m") == "m");
	assert(not rng.parse("n"));

	auto seq = abc > def > ghi;

	assert(seq.parse("abcdefghi")->pick<0>() == "abc");
	assert(seq.parse("abcdefghi")->pick<1>() == "def");
	assert(seq.parse("abcdefghi")->pick<2>() == "ghi");

	assert(not seq.parse("abcdefghX"));

	auto cho = abc | def | ghi;
	assert(*cho.parse("abc") == "abc");
	assert(*cho.parse("def") == "def");
	assert(*cho.parse("ghi") == "ghi");
	assert(not cho.parse("XXX"));

	auto pla = &abc;
	assert(pla.parse("abc"));
	assert(not pla.parse("XXX"));

	auto nla = !abc;
	assert(not nla.parse("abc"));
	assert(nla.parse("XXX"));

	auto opt = -abc > def;
	assert(opt.parse("abcdef"));
	assert(opt.parse("def"));
	assert(not opt.parse("XXX"));

	auto zom = *abc > def;
	assert(zom.parse("abcabcdef"));
	assert(zom.parse("abcabcdef")->pick<0>().size() == 2);
	assert(zom.parse("abcabcdef")->pick<0>()[1] == "abc");
	assert(zom.parse("abcabcdef")->pick<0>().pick<1>() == "abc");
	assert(zom.parse("abcdef"));
	assert(zom.parse("def"));
	assert(not zom.parse("XXX"));

	auto oom = +abc > def;
	assert(oom.parse("abcabcdef"));
	assert(oom.parse("abcdef"));
	assert(not oom.parse("def"));
	assert(not oom.parse("XXX"));

	auto cap = capture(oom);
	assert(cap.parse("abcabcdef---") == "abcabcdef");

	auto act1 = action([](auto matched) {
		assert(matched == ignore);
		return 0;
	});
	assert(*act1.parse("abc") == 0);

	auto act2 = action([](auto matched) {
		assert(matched == "abc");
		return 123;
	});	
	assert(*(abc >= act2).parse("abc") == 123);

	auto prd = predicate([](auto matched) {
		assert(matched == ignore);
		return true;
	});
	assert(prd.parse("abc"));

	auto prd1 = abc > predicate([](auto matched) {
		assert(*matched == "abc");
		return true;
	});

	assert(*prd1.parse("abc") == "abc");

	auto prd0 = abc > predicate([](auto matched) {
		assert(not matched);
		return false;
	});
	assert(not prd0.parse("XXX"));

	auto list = abc.as<"t0">() > (*("+"_L > abc)).as<"ts">();
	assert(list.parse("abc+abc+abc")->pick<"t0">() == "abc");
	assert(list.parse("abc+abc+abc")->pick<"ts">()[0].pick<0>() == "+");

	auto expression = declare<std::string_view, View<std::string_view>>();

	*expression = capture("atom"_L)
		| capture("("_L > ptr(expression) > ")"_L);

	assert(*expression->parse("atom") == "atom");
	assert(*expression->parse("(atom)") == "(atom)");
	assert(*expression->parse("((atom))") == "((atom))");

	auto paramd = [=](auto x, auto y) { return abc > x > y; };
	auto paramTest = paramd(def, ghi);
	assert(paramTest.parse("abcdefghi"));

	std::cout << "done!\n";

	return 0;
}
