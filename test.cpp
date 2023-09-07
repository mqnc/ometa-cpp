
#include "src/ometa.h"

#include <cassert>
#include <iostream>
#include <sstream>

#include <map>

using std::get;

/*
template<class TupType, size_t... I>
void print(std::ostream& os, const TupType& tup, std::index_sequence<I...>){
	os << "(";
	(..., (os << (I == 0? "" : ", ") << std::get<I>(tup)));
	os << ")";
}

template<class... T>
std::ostream& operator<<(std::ostream& os, const std::tuple<T...>& tup){
	print(os, tup, std::make_index_sequence<sizeof...(T)>());
	return os;
}

template<class T>
std::ostream& operator<<(std::ostream& os, const std::deque<T>& deq){
	os << "[";
	std::string sep = "";
	for(const auto& el : deq){
		os << sep << el;
		sep = ", ";
	}
	os << "]";
	return os;
}
*/

int main() {

	assert(ANY.parse("1"));
	assert(not ANY.parse(""));

	auto abc = "abc"_L;
	auto def = "def"_L;
	auto ghi = "ghi"_L;

	auto lit = abc;
	assert(lit.parse("abcd")->value == "abc");
	assert(not lit.parse("abX"));

	auto seq = abc > def > ghi;

	assert(get<0>(seq.parse("abcdefghi")->value).value == "abc");
	assert(get<1>(seq.parse("abcdefghi")->value).value == "def");
	assert(get<2>(seq.parse("abcdefghi")->value).value == "ghi");

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
	assert(get<0>(zom.parse("abcabcdef")->value).value.size() == 2);
	assert(zom.parse("abcdef"));
	assert(zom.parse("def"));
	assert(not zom.parse("XXX"));

	auto oom = +abc > def;
	assert(oom.parse("abcabcdef"));
	assert(oom.parse("abcdef"));
	assert(not oom.parse("def"));
	assert(not oom.parse("XXX"));

	auto cap = CAP(oom);
	assert(cap.parse("abcabcdef---")->value.match == "abcabcdef");

	auto act = abc >= [](auto matched) {
		assert(matched->value == "abc");
		return 123;
	};
	assert(act.parse("abc")->value == 123);

	auto prd1 = abc <= [](auto matched) {
		assert(matched->value == "abc");
		return true;
	};
	assert(prd1.parse("abc")->value == "abc");

	auto prd0 = abc <= [](auto matched) {
		assert(not matched);
		return false;
	};
	assert(not prd0.parse("XXX"));


	Parser check(
		[](auto src, auto ctx) {
			(void) src;

			if constexpr (not ctx.is_empty()) {
				assert(ctx.GET(t0).value == "abc");
				assert(get<1>(ctx.GET(ts).value[1].value).value == "abc");
			}

			return match(empty, src);
		}
	);

	auto list = abc AS(t0) > *("+"_L > abc) AS(ts) > check;
	list.parse("abc+abc+abc");

	auto expression = makeDummy<std::string_view, std::string>();

	expression = ("atom"_L | "("_L > REF(expression) > ")"_L) >= [](auto matched) {
		std::stringstream ss;
		switch (matched->value.index()) {
			case 0:
				ss << get<0>(matched->value).value;
				break;
			case 1: {
				auto tuple = get<1>(matched->value).value;
				ss << get<2>(tuple).value
				   << get<1>(tuple).value
				   << get<0>(tuple).value;
				break;
			}
		}
		return ss.str();
	};

	assert(expression.parse("atom")->value == "atom");
	assert(expression.parse("(atom)")->value == ")atom(");
	assert(expression.parse("((atom))")->value == "))atom((");

	std::cout << "done!\n";
	return 0;
}
