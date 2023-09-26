
## The Agony of Choice

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

## Recursion

It is a bit of a shame that recursion needs extra treatment. It would be nice if that wasn't necessary. Maybe something can be done if all rules are class members with defined return type or something...

Anyway, the first idea was to make the recursive rule mutable, first defining its type, letting it be referenced by other rules and then defining it, possibly depending on those other rules or itself. References are parsers with lambdae that capture an actual reference of the rule and when it changes, the lambdae use the new rule.

I found a bit ugly that references need some special syntax because they need some special treatment but I intended to fix that later.

First syntax looked like this:

```cpp
(R) := {std::string_view} -> {int}; // forward declaration
S := "(" @R ")"; // reference
R :> S; // definition
```

I did a big refactory where all lambdae captured referenced rules by reference and temporary (rvalue) rules by value:

```cpp
A := .;
B := A A; // A is captured by reference
C := "x" | "y"; // the literal parsers (which are not stored in a variable somewhere else) are captured by value
```

This is what std::forward is usually great at, except [it doesn't work with lambda captures](https://vittorioromeo.info/index/blog/capturing_perfectly_forwarded_objects_in_lambdas.html). So I used these wrappers, already unhappy how this inflates the code into very C++esque unintuitive complicatery. To be sure that I used references everywhere, I deleted the copy constructor of the parser class and only allowed move. This made it impossible to use std::function in the forward declaration of recursive rules, as std::functions must be copyable, so they cannot be assigned a lambda that captures uncopyable parsers. C++23 will have [move only functions]{https://en.cppreference.com/w/cpp/utility/functional/move_only_function/move_only_function} which worked but I want to stay C++20 for now. I was thinking to throw inside the copy constructor but I instead decided to screw the whole std::forwardery and copy around parsers like ints everywhere and use std::shared_ptrs instead of references. The code looks just so much better and I can retro justify it by saying it's safe because there won't be any dangling references. Also worked on the syntax a bit, now looks like this:

```cpp
expression' : {std::string} -> {int}; // forward declaration
bracedExpression := "(" expression' ")"; // reference
expression' => primary | bracedExpression; // definition
```

I like how the prime' thing is subtle but visible. I dislike how it messes up C++ syntax highlighting tho and I don't want to implement a proper custom one (which is impossible for github). Maybe I will switch to @expression or expression@...
