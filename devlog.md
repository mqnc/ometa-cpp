
## ToDo

* implement bindings in the syntax
* implement macros
* implement snippets with trees
* update readme
* preserve whitespaces
* selective debug log
* prettify debug log
* handle context
* error handling

* make . <> "" return snippets by default
* have a convenient way for snippet literals in C++ sections
* implement == and != for snippet trees

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

## Recursion := Recursion

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

This is what std::forward is usually great at, except [it doesn't work with lambda captures](https://vittorioromeo.info/index/blog/capturing_perfectly_forwarded_objects_in_lambdas.html). So I used these wrappers, already unhappy how this inflates the code into very C++esque unintuitive complicatery. To be sure that I used references everywhere, I deleted the copy constructor of the parser class and only allowed move. This made it impossible to use std::function in the forward declaration of recursive rules, as std::functions must be copyable, so they cannot be assigned a lambda that captures uncopyable parsers. C++23 will have [move only functions](https://en.cppreference.com/w/cpp/utility/functional/move_only_function/move_only_function) which worked but I want to stay C++20 for now. I was thinking to throw() inside the copy constructor but I instead decided to screw the whole std::forwardery and copy around parsers like ints everywhere and use std::shared_ptrs instead of references. The code looks just so much better and I can retro justify it by saying it's safe because there won't be any dangling references. Also worked on the syntax a bit, now looks like this:

```cpp
expression' : {std::string} -> {int}; // forward declaration
bracedExpression := "(" expression' ")"; // reference
expression' => primary | bracedExpression; // definition
```

I like how the `prime'` thing is subtle but visible. I dislike how it messes up C++ syntax highlighting tho and I don't want to implement a proper custom one (which is impossible for github). Maybe I will switch to `@expression` or `expression@`...

Switched to `expression^` now that `^` is no longer used for actions (see below).

## Whitespace, the Final Frontier

I once heard about a whitespace operator in C++ (was probably a joke) and was thinking it'd be kinda cool to be able to write `a b` for `a*b` like in mathematics. Now that I am actually confronted with one, I must say that it has many annoying implications. Since a whitespace between two expressions in most parsing languages implies a sequence, we are very restricted with parentheses and letting symbols have different meenings as prefix or postfix. For example, the most intuitive syntax for macros (parameterizing rules) would be with appended parentheses, like this:

```cpp
list(X) := X ("," X)*
```

But then if we call that in a rule, it looks like this:

```cpp
identifierList := list(identifier)
```

And here we instead have a sequence of `list` and `(identifier)`:

```cpp
identifierList := list (identifier)
```

Uh oh, that is too similar. In [cpp-peglib](https://github.com/yhirose/cpp-peglib) this is solved by prioritizing macro calls. One wouldn't put a single identifier in parentheses if it's not a macro call. If there are more than one identifiers in there, a comma between them decides whether it's a macro call or a parenthesised sequence. It's ok I guess but it doesn't feel nice. Python would probably solve it by making `A(B)` a sequence and `A(B,)` a macro call but that just gives me eye cancer.

And that's not all. [OMeta/JS](https://github.com/alexwarth/ometa-js/blob/master/bs-ometa-compiler.txt) uses `A?` for optionals and `?(...)` for semantic predicates. And what is `A ? (f)`?

We can solve this by actually making `A(B)` a macro call and `A (B)` a sequence, and similarly making `A? (f)` a sequence of an optional `A` followed by `(f)` and `A ?(f)` an `A` that is accepted if `f` is true (except we will use {} but that's not the point). However, significant whitespace doesn't feel like a good idea somehow. No language I know has significant whitespace, it's probably a whole nother can of worms.

We could also solve it by just using some explicit sequence operator like `A, B`. But [PEG](https://en.wikipedia.org/wiki/Parsing_expression_grammar#Examples), [ANTLR](https://www.antlr.org/) and even the [Dragon Book](https://en.wikipedia.org/wiki/Compilers:_Principles,_Techniques,_and_Tools) use whitespaces for sequences. [EBNF](https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_form) uses commae... But I am actually also a friend of making my grammars reflect whitespaces (in the source they parse) explicitly like this:

```cpp
_ := (" " | "\n" | "\t")*
Addition := Number (_ "+" _ Number)*
```

instead of having identifiers and operators swallow all following whitespaces like in [the original PEG grammar](https://bford.info/pub/lang/peg.pdf). And how would `A, _, B` look compared to `A _ B`!? Ugly, that's how. So I will try to stick to whitespaces and find workarounds for all the problems they cause.

For now:

```cpp
list[X] := X ("," X)*
identifierList := list[identifier]

optional? ^?{return predicate;}
```

"Wait a minute!" I hear you think. "What happened to `[a-z]` ranges?" Let's be honest, they are not as useful in language design as they are when hacking together regular expressions for doing a quick search and replace operation across your codebase. You really only ever need them for specifying numbers and identifiers, maybe some unicode shenanigans. If the range syntax is slightly more elaborate than `[a-z]`, it doesn't really hurt. The `{'a'}..{'z'}` syntax can also express ranges for non text parsers, as long as the source elements can be compared.

## Grand Unified Theory of Actions and Predicates

It would be nice to unify the syntax for actions and predicates somehow. Actions genereate semantic values, predicates decide if parsing continues. The solution right now looks like this:

```cpp
rule1 := A B ^{return fn();} C;
// returns a tree of the four semantic values of A, B, fn() and C

rule2 := A B -> {return fn($1, $2);} C;
// feeds a tree of two semantic values to fn and in the end returns a tree of that result and the value of C

// the implicit whitespace sequence operator and -> have the same precedence, so the above rule is the same as (((A B) -> {...}) C)

rule3 := A B ^?{return fn();} C;
// generates values for A and B, continues parsing if fn() is true and if so returns a tree of the values of A, B and C

rule4 := A B -> ?{return fn($1, $2);} C;
// feeds a tree of two semantic values to fn and if it returns true, returns a tree of three semantic values of A, B and C
```

The appalling `^` only needs to be there so that `B ?{` (B followed by predicate) can not be confused with `B? {` (optional B followed by action). Other considerations for predicates were:

* `{fn()?}` or `{?fn()}` but things in curly braces should be C++.

* `if{fn()}` but it somehow sucks if `if` is the only keyword in the whole rule language.

* `{...}!?`, `{{...}}`, `#{...}`

Technically, all four constructs are very similar and there could be a unified syntax for all of them. We could also make the distinction inside C++, if `{...}` returns a bool, it is a predicate. But we might want to return a bool as a semantic value as well :/

I know! I use `&` again. Look ahead is already kind of an assertion, might as well use it for semantic predicates.

```cpp
rule1 := A B {return fn();} C;
rule2 := A B -> {return fn($1, $2);} C;
rule3 := A B &{return fn();} C;
rule4 := A B -> &{return fn($1, $2);} C;
```

Also, I want to provide the ability to omit `return` and `;` if it's a single expression, so:

```cpp
rule1 := A B {fn()} C;
rule2 := A B -> {fn($1, $2)} C;
rule3 := A B &{fn()} C;
rule4 := A B -> &{fn($1, $2)} C;
```

Found a problem, this doesn't work:

```cpp
predicate := {fn()};
conditional := &predicate something
```

The `&` would confusingly not be an operator but actually part of the `&{}`, the above thing counterintuitively translates to `{fn()}` being an action and `&predicate` always passing.

Had two more ideas:

```cpp
rule1 := A B !{fn()} C;
rule2 := A B -> !{fn($1, $2)} C;
rule3 := A B ?{fn()} C;
rule4 := A B -> ?{fn($1, $2)} C;
```

If there is no `{...}` without any prefix and it's always `!` for actions or `?` for predicates (like in OMeta/JS), the situation is never ambiguous. But I don't like it. Actions should be without extra decorations.

Enough! The only way to prevent people from trying to rip apart the `{...}` and the `?` or `&` is to put the latter into the former. Most other parser MCs use `?` for semantic predicates, so will we. It can go in the beginning or the end but it's easier to parse if it's in the beginning, so `{?fn()}` it is!

Also, there's not really much point in making a difference between `{?...}` and `-> {?...}` as the latter will pipe everything through anyway (you can see in the comments of the first listing in this section that rule3 and rule4 return the same thing). So we will just omit the arrow here. Then we're also very inline with OMeta/JS.

Houston, we have another problem:

```cpp
predicate := {? checkA($)}
rule := A predicate
```

This doesn't work if checkA can't handle `ignore` as input argument. The first line creates a parser that doesn't feed anything (and hence does feed `ignore`) into the lambda that forms the predicate. In the second line, a new parser is created that does feed the semantic value of `A` into the lambda. Even if the predicate is never used without an argument (and hence `ignore` as an argument), the first line still wants to instantiate it for `ignore`.

I already tried making the `Predicate` class not inheriting from `Parser` but implicitly casting to one but this cast doesn't always happen. We can't call a `parse` method on it for instance.

`defer` to the rescue! I already only saw syntactically hideous solutions to this misery, when I clutched to the last straw: When we call the predicate lambda in the predicate parser, we run its argument (`ignore`) through a template that also requires the type of the source code as a template argument. This way, the thing doesn't get instantiated if the source type is not known, which only becomes known when the whole predicate parser is actually put into action without arguments.