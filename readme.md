
# OMeta/C++

Compiler compiler for C++, inspired by [OMeta](https://en.wikipedia.org/wiki/OMeta). Recursive descent [PEG](https://en.wikipedia.org/wiki/Parsing_expression_grammar) parser with extensions for generating output.

Directly embed your parsing rules into C++, transpile with `ometa` to less pretty but pure C++ and then compile it with a proper C++ compiler.

## Project Status

Still very much in development, barely self-hosting.

## Installation

Make sure you have clang installed. If there is the slightest bug in the code, g++ wants to generate ginormous error messages (due to all the nested templates) and crashes. I did not try MSVC.

```
git clone https://github.com/mqnc/ometa-cpp
cd ometa-cpp
mkdir build
cd build
cmake ..
make ometa-cpp
./ometa-cpp ../ometa-cpp.ometa ../test.cpp
diff -s ../ometa-cpp.ometa.cpp ../test.cpp
```

## Why not just use XYZ?

I recommend that you use [cpp-peglib](https://github.com/yhirose/cpp-peglib), [PEGTL](https://github.com/taocpp/PEGTL), [lexy](https://lexy.foonathan.net/) or [Boost Spirit](https://github.com/boostorg/spirit) instead. Those are all way more battle-tested. They also served a lot as inspiration, kudos to them.

Main focus in the development was for this to be a fun hobby project and for things to be done in a simple and proper-feeling way. In most other projects, the parse functions return a bool, values are computed separately and the functions get read/write access to some global context. Here, parse functions return `std::optional<Match>`, where `Match` contains the semantic value and a source pointer to the end of the match. Context can be extended and passed on to nested parse calls as readonly, so backtracking is automatically solved. Also, everything is templated and typesafe. Your source doesn't have to be a string, it can be any kind of sequence of things (that is a [`std::forward_range`](https://en.cppreference.com/w/cpp/ranges/forward_range)).

## Limitations

* no left recursion
* Proper-feelingness probably comes at the cost of performance.
* Development in this template jungle is a hell of torture and despair, error messages by the compiler span pages and are mostly not helpful.
* Judging from experience, I will probably lose interest in this project soon and then it's no longer maintained. But at this point, it is probably complete and free of bugs 😌.

## Usage

```cpp
// rule:
myRule := ...;

// prioritized choice:
myChoice := alternative1 | alternative2 | alternative3;

// sequence:
mySequence := thing1 thing2 thing3;

// look ahead:
myPeek := &thisMustFollow;
myPoke := !thisMustNotFollow;

// repetition:
myRepetition := optional? zeroOrMore* oneOrMore+;

// string literals:
myString := "abc \"quoted\" \\backslash \nnew line \ttab";

// ranges:
myRange := {'A'}..{'Z'};

// any:
myAny := .;

// epsilon (always matches, consumes no input):
myEpsilon := ();

// generating semantic values:
myValue := ^myLambda;
myInlineValue := ^{return std::string("awa");};
myDependentValue := A B C -> {return $1 + $2 + $3;}

// semantic predicates:
myPredicate := ^?myLambda;
myInlinePredicate := ^?{return std::rand() % 2 == 0;};
myDependentPredicate := A B C -> ?{return $1 + $2 + $3 > 10;};

// ignore values:
myPick := ~ignoreMe useJustMe ~ignoreMe -> {return $;};

// capture source:
myCapture := <A B C>;

// recursion:
expression' : {std::string} -> {int}; // forward declaration
bracedExpression := "(" expression' ")"; // reference
expression' => primary | bracedExpression; // definition

// bindings (todo):
myBinding := firstThing:x secondThing:y -> {return $x + $y;};

// macros (todo):
myList[thing, sep] := thing (sep thing)*;
myAddition := myList[number, plus];
```

* things in `{curly braces}` are written in C++ (with some extensions like `$` for `value`);
* all alternatives in a prioritized choice must return the same type
* see `ometa.ometa` for details, it describes itself

## Contribute

Feel free to report issues and suggest suggestions!