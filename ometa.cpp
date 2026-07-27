

#include "ometa.h"

#include <cassert>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <ctime>

using ViewTree = ometa::ViewTree<std::string_view>;

int main(int argc, char* argv[]) {

	if(argc != 3){
		std::cout << "usage: " << argv[0] << " INPUT_FILE OUTPUT_FILE\n";
		return EXIT_FAILURE;
	}

	const auto comment = ometa::rule<"comment">(ometa::capture(~"/*"_lit_ > *(!~"*/"_lit_ > ~ometa::any()) > ~"*/"_lit_)| ometa::capture(~"//"_lit_ > *(!~"\n"_lit_ > ~ometa::any()) > ~"\n"_lit_));

	const auto whitespace = ometa::rule<"whitespace">(ometa::capture(*(" "_lit_| "\t"_lit_| "\n"_lit_| comment)));
	const auto _ = ometa::rule<"_">(~whitespace);

	const auto identStart = ometa::rule<"identStart">(ometa::range(('A'), ('Z'))| ometa::range(('a'), ('z'))| "_"_lit_| "::"_lit_);
	const auto identContinue = ometa::rule<"identContinue">(identStart| ometa::range(('0'), ('9')));
	const auto identifier = ometa::rule<"identifier">(ometa::capture(identStart > *identContinue));
	const auto reference = ometa::rule<"reference">(identifier > ~"^"_lit_ >= ometa::action([](auto value, auto& context){return "ometa::ptr("_tree_ + value + ")"_tree_;}));

	const auto cppChar = ometa::rule<"cppChar">(ometa::capture(~"\\"_lit_ > ~ometa::any()| ~ometa::any()));
	const auto cppLiteral = ometa::rule<"cppLiteral">(ometa::capture(~"\'"_lit_ > ~*(!"'"_lit_ > ~cppChar) > ~"\'"_lit_| ~"\""_lit_ > ~*(!"\""_lit_ > ~cppChar) > ~"\""_lit_));
	
	const auto viewTreeLiteral = ometa::rule<"viewTreeLiteral">(~"`"_lit_ > ometa::action([](auto value, auto& context){return "\""_tree_;}) > *(~"\""_lit_ >= ometa::action([](auto value, auto& context){return "\\\""_tree_;})
        | !~"`"_lit_ > cppChar) > ~"`"_lit_ > ometa::action([](auto value, auto& context){return "\"_tree_"_tree_;}) >= ometa::concat);

	const auto valueReference = ometa::rule<"valueReference">(~"$"_lit_ >= ometa::action([](auto value, auto& context){return "value"_tree_;}));
	const auto indexedValueReference = ometa::rule<"indexedValueReference">(~"$"_lit_ > ometa::capture(+ometa::range(('0'), ('9'))) >= ometa::action([](auto value, auto& context){return "ometa::pick<"_tree_ + value + ">(value)"_tree_;}));
	const auto taggedValueReference = ometa::rule<"taggedValueReference">(~"$"_lit_ > identifier >= ometa::action([](auto value, auto& context){return "ometa::pick<\""_tree_ + value + "\">(value)"_tree_;}));

	auto cppExpression = ometa::declare<std::string_view, ViewTree>();
	const auto parenthesizedCppExpression = ometa::rule<"parenthesizedCppExpression">(~"("_lit_ > ometa::ptr(cppExpression) > ~")"_lit_);
	const auto bracketedCppExpression = ometa::rule<"bracketedCppExpression">(~"["_lit_ > ometa::ptr(cppExpression) > ~"]"_lit_);
	const auto bracedCppExpression = ometa::rule<"bracedCppExpression">(~"{"_lit_ > ometa::ptr(cppExpression) > ~"}"_lit_);
	const auto predicateCppExpression = ometa::rule<"predicateCppExpression">(~"{"_lit_ > _ > ~"?"_lit_ > ometa::ptr(cppExpression) > ~"}"_lit_);

	const auto contextTableDeclaration = ometa::rule<"contextTableDeclaration">(identifier > _ > ~":"_lit_ > _ > bracedCppExpression > _ > ~"->"_lit_ > _ > bracedCppExpression >= ometa::action([](auto value, auto& context){return 
			"\tometa::makeTagged<\""_tree_ + ometa::pick<0>(value) + "\">"_tree_
			+ "(ometa::ContextTable<"_tree_ + ometa::pick<1>(value) + ", "_tree_ + ometa::pick<2>(value) + ">{})"_tree_
		;}));
	const auto contextValueDeclaration = ometa::rule<"contextValueDeclaration">(identifier > _ > ~":"_lit_ > _ > bracedCppExpression > -(_ > ~"="_lit_ > _ > bracedCppExpression) >= ometa::action([](auto value, auto& context){return 
			"\tometa::makeTagged<\""_tree_ + ometa::pick<0>(value) + "\">"_tree_
			+ "(ometa::ContextValue<"_tree_ + ometa::pick<1>(value) + ">{"_tree_
			+ (ometa::pick<2>(value).size() > 0 ? ometa::pick<2>(value)[0] : ""_tree_) + "})"_tree_
		;}));
	const auto contextItemDeclaration = ometa::rule<"contextItemDeclaration">(contextTableDeclaration| contextValueDeclaration);
	const auto contextItemDeclarationList = ometa::rule<"contextItemDeclarationList">(contextItemDeclaration > _ > *(","_lit_ > ometa::action([](auto value, auto& context){return "\n"_tree_;}) > _ > contextItemDeclaration) >= ometa::concat);
	const auto contextDeclaration = ometa::rule<"contextDeclaration">(identifier > ~"@"_lit_ > _ > ~":"_lit_ > _ > contextItemDeclarationList > _ > ~";"_lit_ >= ometa::action([](auto value, auto& context){return "auto "_tree_ + ometa::pick<0>(value) + " = ometa::Context(\n"_tree_ + ometa::pick<1>(value) + "\n);"_tree_;}));

	const auto contextReference = ometa::rule<"contextReference">(~"@"_lit_ > identifier >= ometa::action([](auto value, auto& context){return "ometa::pick<\""_tree_ + value + "\">(context)"_tree_;}));
	const auto outsideContextReference = ometa::rule<"outsideContextReference">(identifier > ~"@"_lit_ > identifier >= ometa::action([](auto value, auto& context){return "ometa::pick<\""_tree_ + ometa::pick<1>(value) + "\">("_tree_ + ometa::pick<0>(value) + ")"_tree_;}));

	*cppExpression = ometa::rule<"cppExpression">(*(identifier > ometa::predicate([](auto value, auto& context){return  value && *value != "return";})| contextReference| viewTreeLiteral| cppLiteral| parenthesizedCppExpression >= ometa::action([](auto value, auto& context){return "("_tree_ + value + ")"_tree_;})
		| bracketedCppExpression >= ometa::action([](auto value, auto& context){return "["_tree_ + value + "]"_tree_;})
		| bracedCppExpression >= ometa::action([](auto value, auto& context){return "{"_tree_ + value + "}"_tree_;})
		| indexedValueReference| taggedValueReference| valueReference| !")"_lit_ > !"]"_lit_ > !"}"_lit_ > !";"_lit_ > ometa::any()) >= ometa::concat);

	auto cppCode = ometa::declare<std::string_view, ViewTree>();
	const auto parenthesizedCppCode = ometa::rule<"parenthesizedCppCode">(~"("_lit_ > ometa::ptr(cppCode) > ~")"_lit_);
	const auto bracketedCppCode = ometa::rule<"bracketedCppCode">(~"["_lit_ > ometa::ptr(cppCode) > ~"]"_lit_);
	const auto bracedCppCode = ometa::rule<"bracedCppCode">(~"{"_lit_ > ometa::ptr(cppCode) > ~"}"_lit_);
	const auto predicateCppCode = ometa::rule<"predicateCppCode">(~"{"_lit_ > _ > ~"?"_lit_ > ometa::ptr(cppCode) > ~"}"_lit_);

	auto ruleForwardDecl = ometa::declare<std::string_view, ViewTree>();
	auto ruleDefinition = ometa::declare<std::string_view, ViewTree>();
	auto ruleRedefinition = ometa::declare<std::string_view, ViewTree>();
	auto macroDefinition = ometa::declare<std::string_view, ViewTree>();

	*cppCode = ometa::rule<"cppCode">(*(ometa::ptr(ruleForwardDecl)| ometa::ptr(ruleDefinition)| ometa::ptr(ruleRedefinition)| ometa::ptr(macroDefinition)| contextDeclaration| contextReference| outsideContextReference| identifier| viewTreeLiteral| cppLiteral| parenthesizedCppCode >= ometa::action([](auto value, auto& context){return "("_tree_ + value + ")"_tree_;})
		| bracketedCppCode >= ometa::action([](auto value, auto& context){return "["_tree_ + value + "]"_tree_;})
		| bracedCppCode >= ometa::action([](auto value, auto& context){return "{"_tree_ + value + "}"_tree_;})
		| indexedValueReference| taggedValueReference| valueReference| !")"_lit_ > !"]"_lit_ > !"}"_lit_ > ometa::any()) >= ometa::concat);

	const auto any = ometa::rule<"any">("."_lit_ >= ometa::action([](auto value, auto& context){return "ometa::any()"_tree_;}));
	const auto epsilon = ometa::rule<"epsilon">("()"_lit_ >= ometa::action([](auto value, auto& context){return "ometa::epsilon()"_tree_;}));

	const auto literalCharacter = ometa::rule<"literalCharacter">(ometa::capture("\\"_lit_ > ("n"_lit_| "r"_lit_| "t"_lit_| "'"_lit_| "\""_lit_| "\\"_lit_))| "\""_lit_ >= ometa::action([](auto value, auto& context){return "\\\""_tree_;})
		| ometa::capture(!"\\"_lit_ > ometa::any()));

	const auto ignoredLiteral = ometa::rule<"ignoredLiteral">(~"\'"_lit_ > ometa::action([](auto value, auto& context){return "~\""_tree_;}) > *(!"'"_lit_ > literalCharacter) > ~"\'"_lit_ > ometa::action([](auto value, auto& context){return "\"_lit_"_tree_;}) >= ometa::concat);
	const auto capturedLiteral = ometa::rule<"capturedLiteral">(ometa::capture("\""_lit_ > *(!"\""_lit_ > literalCharacter) > "\""_lit_) > ometa::action([](auto value, auto& context){return "_lit_"_tree_;}) >= ometa::concat);
	const auto literal = ometa::rule<"literal">(ignoredLiteral| capturedLiteral);

	const auto range = ometa::rule<"range">(bracedCppCode > _ > ~".."_lit_ > _ > bracedCppCode >= ometa::action([](auto value, auto& context){return 
		"ometa::range(("_tree_ + ometa::pick<0>(value) + "), ("_tree_ + ometa::pick<1>(value) + "))"_tree_
	;}));

	auto expression = ometa::declare<std::string_view, ViewTree>();
	const auto parenthesized = ometa::rule<"parenthesized">(~"("_lit_ > _ > ometa::ptr(expression) > _ > ~")"_lit_ >= ometa::action([](auto value, auto& context){return "("_tree_ + value + ")"_tree_;}));
	const auto capture = ometa::rule<"capture">(~"<"_lit_ > _ > ometa::ptr(expression) > _ > ~">"_lit_ >= ometa::action([](auto value, auto& context){return "ometa::capture("_tree_ + value + ")"_tree_;}));

	const auto action = ometa::rule<"action">(identifier| bracedCppExpression >= ometa::action([](auto value, auto& context){return "ometa::action([](auto value, auto& context){return "_tree_ + value + ";})"_tree_;})
		| bracedCppCode >= ometa::action([](auto value, auto& context){return "ometa::action([](auto value, auto& context){"_tree_ + value + "})"_tree_;}));
	const auto predicate = ometa::rule<"predicate">(identifier| predicateCppExpression >= ometa::action([](auto value, auto& context){return "ometa::predicate([](auto value, auto& context){return "_tree_ + value + ";})"_tree_;})
		| predicateCppCode >= ometa::action([](auto value, auto& context){return "ometa::predicate([](auto value, auto& context){"_tree_ + value + "})"_tree_;}));

	const auto parameterizedAction = ometa::rule<"parameterizedAction">(~"->"_lit_ > ometa::action([](auto value, auto& context){return " >= "_tree_;}) > _ > action >= ometa::concat);

	const auto macroCall = ometa::rule<"macroCall">(identifier > _ > ~"["_lit_ > ometa::action([](auto value, auto& context){return "("_tree_;}) > _ > ometa::ptr(expression) > _ > *(","_lit_ > ometa::action([](auto value, auto& context){return " "_tree_;}) > _ > ometa::ptr(expression)) > _ > ~"]"_lit_ > ometa::action([](auto value, auto& context){return ")"_tree_;}) >= ometa::concat);

	const auto primary = ometa::rule<"primary">(reference| macroCall| any| epsilon| literal| range| capture| predicate| action| parenthesized);

	const auto optional = ometa::rule<"optional">("?"_lit_ >= ometa::action([](auto value, auto& context){return "-"_tree_;}));
	const auto zeroOrMore = ometa::rule<"zeroOrMore">("*"_lit_ >= ometa::action([](auto value, auto& context){return "*"_tree_;}));
	const auto oneOrMore = ometa::rule<"oneOrMore">("+"_lit_ >= ometa::action([](auto value, auto& context){return "+"_tree_;}));
	const auto repetition = ometa::rule<"repetition">(optional| zeroOrMore| oneOrMore);
	const auto tag = ometa::rule<"tag">(~":"_lit_ > _ > identifier);

	const auto postfixed = ometa::rule<"postfixed">(primary > _ > -tag > _ > -repetition > _ > -tag >= ometa::action([](auto value, auto& context){ 
			auto result = ometa::pick<0>(value);
			if (ometa::pick<1>(value).size() == 1) {result = "tagResult<\""_tree_ + ometa::pick<1>(value)[0] + "\">("_tree_ + result + ")"_tree_;}
			if (ometa::pick<2>(value).size() == 1) {result = ometa::pick<2>(value)[0] + result;}
			if (ometa::pick<3>(value).size() == 1) {result = "tagResult<\""_tree_ + ometa::pick<3>(value)[0] + "\">("_tree_ + result + ")"_tree_;}
			return result;
		}));

	const auto prefixed = ometa::rule<"prefixed">(ometa::capture(-("&"_lit_| "!"_lit_| "~"_lit_)) > _ > postfixed >= ometa::concat);

	const auto sequence = ometa::rule<"sequence">(prefixed > *(_ > (ometa::action([](auto value, auto& context){return " > "_tree_;}) > prefixed >= ometa::concat
			| parameterizedAction)) >= ometa::concat);

	const auto choice = ometa::rule<"choice">(sequence > *(ometa::capture(whitespace > "|"_lit_ > whitespace) > sequence) >= ometa::concat);

	*expression = ometa::rule<"expression">(choice);

	*ruleForwardDecl = ometa::rule<"ruleForwardDecl">(identifier > ~"^"_lit_ > _ > ~":"_lit_ > _ > bracedCppExpression > _ > ~"->"_lit_ > _ > bracedCppExpression > _ > ~";"_lit_ >= ometa::action([](auto value, auto& context){return 
			"auto "_tree_ + ometa::pick<0>(value) + " = ometa::declare<"_tree_
				+ ometa::pick<1>(value) + ", "_tree_ + ometa::pick<2>(value) + ">();"_tree_
		;}));

	*ruleDefinition = ometa::rule<"ruleDefinition">(identifier > _ > ~":="_lit_ > _ > ometa::ptr(expression) > _ > ~";"_lit_ >= ometa::action([](auto value, auto& context){return "const auto "_tree_ + ometa::pick<0>(value) + " = ometa::rule<\""_tree_ + ometa::pick<0>(value) + "\">("_tree_ + ometa::pick<1>(value) + ");"_tree_;}));

	*ruleRedefinition = ometa::rule<"ruleRedefinition">(identifier > ~"^"_lit_ > _ > ~"=>"_lit_ > _ > ometa::ptr(expression) > _ > ~";"_lit_ >= ometa::action([](auto value, auto& context){return "*"_tree_ + ometa::pick<0>(value) + " = ometa::rule<\""_tree_ + ometa::pick<0>(value) + "\">("_tree_ + ometa::pick<1>(value) + ");"_tree_;}));

	const auto macroParameterList = ometa::rule<"macroParameterList">(~"["_lit_ > _ > ometa::action([](auto value, auto& context){return "auto "_tree_;}) > identifier > _ > *(~","_lit_ > _ > identifier >= ometa::action([](auto value, auto& context){return ", auto "_tree_ + value;})) > _ > ~"]"_lit_ >= ometa::concat);
	*macroDefinition = ometa::rule<"macroDefinition">(identifier > _ > macroParameterList > _ > ~":="_lit_ > _ > ometa::ptr(expression) > _ > ~";"_lit_ >= ometa::action([](auto value, auto& context){return 
			"const auto "_tree_ + ometa::pick<0>(value) + " = [=]("_tree_ + ometa::pick<1>(value) + "){return "_tree_ + ometa::pick<2>(value) + ";};"_tree_
		;}));

	auto code = ometa::readFile(argv[1]);

	auto result = cppCode->parse(code);
	if (result) {
		try {
			auto backup = ometa::readFile(argv[2]);
			std::time_t time = std::time({});
			char timeString[std::size("yyyy_mm_dd__hh_mm_ssZ")];
			std::strftime(std::data(timeString), std::size(timeString),
				"%Y_%m_%d__%H_%M_%S", std::gmtime(&time));
			ometa::writeFile(std::string(argv[2]) + "." + timeString + ".backup", backup);
		}
		catch (...) {}

		ometa::writeFile(argv[2], *result);
	}
	else {
		std::cout << "fail\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
