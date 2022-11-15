
#include "src/ometa.h"

#include <cassert>
#include <iostream>

using std::get;

int main() {

	auto abc = "abc"_L;
	auto def = "def"_L;
	auto ghi = "ghi"_L;

	auto lit = abc;
	assert(lit.parse("abcd")->value == "abc");
	assert(not lit.parse("abX"));

	auto seq = abc > def > ghi;
	assert(get<0>(seq.parse("abcdefghi")->value) == "abc");
	assert(get<1>(seq.parse("abcdefghi")->value) == "def");
	assert(get<2>(seq.parse("abcdefghi")->value) == "ghi");

	assert(not seq.parse("abcdefghX"));

	auto cho = abc | def | ghi;
	assert(cho.parse("abc")->value.index() == 0);
	assert(cho.parse("def")->value.index() == 1);
	assert(cho.parse("ghi")->value.index() == 2);
	assert(not cho.parse("XXX"));

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
	assert(get<0>(zom.parse("abcabcdef")->value).size() == 2);
	assert(zom.parse("abcdef"));
	assert(zom.parse("def"));
	assert(not zom.parse("XXX"));

	auto oom = +abc > def;
	assert(oom.parse("abcabcdef"));
	assert(oom.parse("abcdef"));
	assert(not oom.parse("def"));
	assert(not oom.parse("XXX"));

	auto act = abc >= [](auto val) {
		assert(val == "abc");
		return 123;
	};
	assert(act.parse("abc")->value == 123);

	auto prd1 = abc <= [](auto val) {
		assert(val->value == "abc");
		return true;
	};
	assert(prd1.parse("abc")->value == "abc");

	auto prd0 = abc <= [](auto val) {
		assert(not val);
		return false;
	};
	assert(not prd0.parse("XXX"));

	std::cout << "done!\n";

	return 0;
}
