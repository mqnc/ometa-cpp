
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
make ometa
./ometa
make ometa
./ometa
```

Note that the first call to `./ometa` regenerates `ometa.ometa.cpp` from `ometa.ometa` again, so you can compile again.

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
myValue := ^{return std::string("awa");};
myDependentValue := A B C -> {return $1 + $2 + $3;}

// semantic predicates:
myPredicate := ^?{return std::rand() % 2 == 0;};
myDependentPredicate := A B C -> ^{return $1 + $2 + $3 > 10;};

// ignore values:
myPick := ~ignoreMe useJustMe ~ignoreMe -> {return $;};

// capture source:
myCapture := <A B C>;

// recursion:
(expression) := {std::string} -> {int}; // forward declaration
bracedExpression := "(" @expression ")"; // reference
expression :> primary | bracedExpression; // definition

// bindings (todo):
myBinding := firstThing:x secondThing:y -> {return $x + $y;};

// macros (todo):
myList[thing, sep] := thing (sep thing)*;
myAddition := myList[number, plus];
```

* things in `{curly braces}` are written in C++ (with some extensions like `$` for `value`);
* all alternatives in a prioritized choice must return the same type
* see `ometa.ometa` for details, it describes itself

## Devlog

### The Agony of Choice

First I implemented the prioritized choice so that `A | B | C` returns a `std::variant<TypeA, TypeB, TypeC>`. The choice factory became huge and ugly, mainly but not only because it should return `std::variant<TypeA, TypeB, TypeC>` instead of `std::variant<std::variant<TypeA, TypeB>, TypeC>`. However, the idiomatic way to deal with variants is to dispatch on them with `std::visit`, then you might as well handle each option right away before merging them with `|`:

```cpp
myChoice :=
	number -> {return std::to_string($);}
	| givenName familyName -> {return std::string($1) + std::string($2);};
```

However, this makes development a bit more difficult because you probably want to write the parser first before generating any values, and most likely the types won't match. In that case, I recommend to ignore all values first:

```cpp
myChoice := ~number | ~(givenName familyName);
```
