
#include "src/tree.h"
#include <iostream>

#include <tuple>

int main() {

	auto t1 = join(makeTagged<"1">(1), makeTagged<"2">(2), makeTagged<"3">(3));
	auto t2 = join(makeTagged<"1">(11), makeTagged<"2">(22), makeTagged<"33">(33));
	auto t3 = join(t1, t2);
	
	std::cout << get<2>(t3);
	std::cout << get<"33">(t3);

	return 0;
}
