
#include "src/ometa.h"

#include <cassert>
#include <iostream>

int main() {

	auto abc = "abc"_L;
	auto def = "def"_L;
	auto ghi = "ghi"_L;

	auto lit = abc;
	assert(lit.parse("abcd"s)->value.copyInto<std::string>() == "abc");
	assert(lit.parse("abX"s) == fail);

	auto seq = abc > def > ghi;
	assert(std::get<0>(seq.parse("abcdefghi"s)->value).copyInto<std::string>() == "abc");
	assert(std::get<1>(seq.parse("abcdefghi"s)->value).copyInto<std::string>() == "def");
	assert(std::get<2>(seq.parse("abcdefghi"s)->value).copyInto<std::string>() == "ghi");

	assert(seq.parse("abcdefghX"s) == fail);

	auto cho = abc | def | ghi;
	assert(cho.parse("abc"s)->value.index() == 0);
	assert(cho.parse("def"s)->value.index() == 1);
	assert(cho.parse("ghi"s)->value.index() == 2);
	assert(cho.parse("XXX"s) == fail);

	auto pla = &abc;
	assert(pla.parse("abc"s).has_value());
	assert(pla.parse("XXX"s) == fail);

	auto nla = !abc;
	assert(nla.parse("abc"s) == fail);
	assert(nla.parse("XXX"s).has_value());

	auto opt = ~abc > def;
	assert(opt.parse("abcdef"s).has_value());
	assert(opt.parse("def"s).has_value());
	assert(opt.parse("XXX"s) == fail);

	auto zom = *abc > def;
	assert(zom.parse("abcabcdef"s).has_value());
	assert(std::get<0>(zom.parse("abcabcdef"s)->value).size() == 2);
	assert(zom.parse("abcdef"s).has_value());
	assert(zom.parse("def"s).has_value());
	assert(zom.parse("XXX"s) == fail);

	auto oom = +abc > def;
	assert(oom.parse("abcabcdef"s).has_value());
	assert(oom.parse("abcdef"s).has_value());
	assert(oom.parse("def"s) == fail);
	assert(oom.parse("XXX"s) == fail);

	// auto act = abc >= [](auto val) {
	// 	assert(val.template copyInto<std::string>() == "abc");
	// 	return 123;
	// };
	// assert(act.parse("abc"s)->value == 123);

	// auto prd1 = abc <= [](auto val) {
	// 	assert(val->value.template copyInto<std::string>() == "abc");
	// 	return true;
	// };
	// assert(prd1.parse("abc"s)->value.copyInto<std::string>() == "abc");

	// auto prd0 = abc <= [](auto val) {
	// 	assert(val == fail);
	// 	return false;
	// };
	// assert(prd0.parse("XXX"s) == fail);

	std::cout << "done!\n";

	return 0;
}
