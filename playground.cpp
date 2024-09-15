#include <iostream>
#include "include/context.h"  // Assume your ContextValue, ContextTable, and Context are here

int main() {
    using namespace ometa;
	using namespace std;

	auto myContext = Context(
		makeTagged<"line">(ContextValue<int>{}),
		makeTagged<"column">(LoggingContextValue<int>{}),
		makeTagged<"symbols">(ContextTable<string, string>{})
	);

	get<"line">(myContext) = 101;
	get<"column">(myContext) = 1001;
	get<"symbols">(myContext).insert({"a", "10001"});

	auto version = myContext.backup();

	std::cout << "\nInitial versions:" << std::endl;
	std::cout << *get<"line">(myContext) << std::endl;
	std::cout << *get<"column">(myContext) << std::endl;
	std::cout << get<"symbols">(myContext).at("a") << std::endl;

	get<"line">(myContext) = 202;
	get<"column">(myContext) = 2002;
	get<"symbols">(myContext).insert({"a", "20002"});
	get<"symbols">(myContext).insert({"a", "20022"});
	get<"symbols">(myContext).insert({"b", "22222"});

	std::cout << "\nModified versions:" << std::endl;
	std::cout << *get<"line">(myContext) << std::endl;
	std::cout << *get<"column">(myContext) << std::endl;
	std::cout << get<"symbols">(myContext).at("a") << std::endl;
	std::cout << get<"symbols">(myContext).at("b") << std::endl;

	myContext.backtrack(version);

	std::cout << "\nrestored versions:" << std::endl;
	std::cout << *get<"line">(myContext) << std::endl;
	std::cout << *get<"column">(myContext) << std::endl;
	std::cout << get<"symbols">(myContext).at("a") << std::endl;

    return 0;
}
