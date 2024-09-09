#include <iostream>
#include "include/context.h"  // Assume your ContextValue, ContextTable, and Context are here

void fn(auto);

int main() {
    using namespace ometa;
	using namespace std;

	auto myContext = Context(
		makeTagged<"line">(ContextValue<int>{}),
		makeTagged<"column">(LoggingContextValue<int>{}),
		makeTagged<"symbols">(ContextTable<string, string>{})
	);

	myContext.get<"line">() = 101;
	myContext.get<"column">() = 1001;
	myContext.get<"symbols">().insert({"a", "10001"});

	cout << "Initial versions:" << endl;
	cout << *(myContext.get<"line">()) << endl;
	cout << *(myContext.get<"column">()) << endl;
	cout << myContext.get<"symbols">().at("a") << endl;

	auto version = myContext.getVersion();

	myContext.get<"line">() = 202;
	myContext.get<"column">() = 2002;
	myContext.get<"symbols">().insert({"a", "20002a"});
	myContext.get<"symbols">().insert({"b", "20002b"});

	// auto version = myContext.getVersion();


    // // Create a Context with Tagged values, using your existing Tagged utility
    // auto myContext = Context(
    //     makeTagged<"value1">(cv1),
    //     makeTagged<"value2">(cv2),
    //     makeTagged<"table1">(ct1),
    //     makeTagged<"regularInt">(1234) // A regular integer, no getVersion/backtrack
    // );

    // // Get the current versions
    // auto versionTuple = myContext.getVersion();
    
    // // Display the current version of each element
    // std::cout << "Initial versions:" << std::endl;
    // std::cout << "ContextValue 1 version: " << std::get<0>(versionTuple) << std::endl;
    // std::cout << "ContextValue 2 version: " << std::get<1>(versionTuple) << std::endl;
    // std::cout << "ContextTable 1 version: " << std::get<2>(versionTuple) << std::endl;
    // std::cout << "Regular int value: " << std::get<3>(versionTuple) << std::endl;

    // // Modify ContextValue and ContextTable instances
    // cv1.setValue(50, 10);  // Change cv1 value to 50, version to 10
    // cv2.setValue(200, 20); // Change cv2 value to 200, version to 20
    // ct1 = ContextTable("UpdatedTable", 30); // Change ct1 data and version

    // std::cout << "\nModified versions:" << std::endl;
    // std::cout << "ContextValue 1 value: " << cv1.getValue() << ", version: " << cv1.getVersion() << std::endl;
    // std::cout << "ContextValue 2 value: " << cv2.getValue() << ", version: " << cv2.getVersion() << std::endl;
    // std::cout << "ContextTable 1 value: " << "UpdatedTable" << ", version: " << ct1.getVersion() << std::endl;

    // // Backtrack to the previous version
    // myContext.backtrack(versionTuple);

    // // Check the values after backtracking
    // std::cout << "\nAfter backtracking:" << std::endl;
    // std::cout << "ContextValue 1 value: " << cv1.getValue() << ", version: " << cv1.getVersion() << std::endl;
    // std::cout << "ContextValue 2 value: " << cv2.getValue() << ", version: " << cv2.getVersion() << std::endl;
    // std::cout << "ContextTable 1 value: " << "UpdatedTable" << ", version: " << ct1.getVersion() << std::endl;
    
    return 0;
}
