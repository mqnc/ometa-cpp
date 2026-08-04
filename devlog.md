
# DevLog

I'm gonna write down my trains of thought here so once this project is super famous, people can understand certain decisions. Also so I can understand certain decisions once I pick the project back up after five years of losing interest. The individual thought trains are chronological within themselves but it can happen that a decision that is noted in the middle of one train overthrows a final decision of another train somewhere else (below or above) in the document. So the readme and the test should be the reference. 

## ToDo

### Most Pressing

* I can also have some Meta class on each Parser instead of just a tag containing log thresholds and rule name
* needs error handling and line+column tracking

### Misc

Next steps would be to rewrite all the examples using all the new features (mainly bindings and context) and also implement some famous parsers, mainly json, json5, lua5.3, g++ or clang ast output, write a minimal C++ formatter.

* the return and semicolon distinction in semantic values sucks
* global log threshold state is not thread-safe; should maybe become part of context; context should maybe have a standard part and a custom part inheriting from it
* consistent naming with log vs debug vs verbose
* see if recursion is properly log-wrapped
* dislike: postfixed := primary _ tag? _ repetition? _ tag?
* cant declare context type for recursive parsers yet
* preserve whitespaces
* selective debug log: we can use numbers as a breakpoint marker, identifiers must not start with them
* prettify debug log
* handle context
* error handling
* UTF8
* cpp comments
* maybe propagate an ignore_value flag (or maybe not, we might want the side effects)
* do some projects like a lua, clang and json5 parser, note errors and catch them with awesome eigen error reports
* memoize (aka packrat parsing); however, need to be aware that context can change parsing result
* parse and summarize clang error output
* make repetition count templated so we can log oneOrMore, zeroOrMore and optional or maybe make optional separate

```
binding := abc:t0 ("+" abc)*:ts;
binding_ts_0_0 := binding -> {$ts[0]} -> {$0};
```
bit ugly that we dont have a syntactically sugary way to use pick on something other than $

```
{@column.set(@column.get()+1); return $0;};
```
sucks that we have to return something here; just `{@column.set(@column.get()+1)}` will be interpreted as needing to be returned

we cant have manual line and column management...

## To Capture or not to Capture

'' and "" are very similar and have identical meaning in Python and PEG. Currently "abc" captures and 'abc' does not, which works against intuition and might be hard to remember. A solution would be that all capture needs to use <> explicitly (including or excluding ranges and any).

Only <> captures:
- very consistent and clear divide between parsing and producing
- any and range are very rare, I often enclose them in <> already anyway
- literals are mostly control keywords whose value should be ignored anyway
- no tilde clutter

Literals ignore but ranges and any capture:
- each of them return what they are probably most used for
- we have a way to capture individual characters, not just spans

"" for captured literals and '' for ignored literals:
- most compact syntax

Everything captures:
- probably most expected

## Putting Things into Context

Contexts are like global storage for a parser. It gets handed down to subparsers and everyone can make changes to it. If the parser backtracks, so will the context. This is implemented via a `snapshot = backup()` and a `backtrack(snapshot)` method on the context classes.

Contexts worked in the end but the syntax is still a bit ugly and maybe it's better if it was like this:

@ := {
	variables: string->string;
	line: int;
};

context: @{
    variables: string->string;
    line: int;
}

or something.
updated this, see tests.

I now have the current options for context:
* PersistentContextValue: a singular value that gets permanently overwritten with every successful parse and does not backtrack
* ContextValue: a singular value that returns itself upon call to backup() and that resets itself upon call to backtrack()
* LoggingContextValue: a singular value that contains a stack, pushing new values onto it and popping when backtracking
* ContextTable: a map of stacks
* Context: a tuple of other context types

Thing is, why would I ever need a stack embedded inside the context when the parser can store the actual value via snapshot on the actual program stack? Uuuuh, because the table would be much more expensive in terms of storage and copying if I always copied the whole table.

## The Agony of Choice

First I implemented the prioritized choice so that `A | B | C` returns a `std::variant<TypeA, TypeB, TypeC>`. The choice factory became huge and ugly, mainly but not only because it should return `std::variant<TypeA, TypeB, TypeC>` instead of `std::variant<std::variant<TypeA, TypeB>, TypeC>`. However, the idiomatic way to deal with variants is to dispatch on them with `std::visit`, then you might as well handle each option right away before merging them with `|`:

```d
myChoice :=
	number -> {return std::to_string($);}
	| givenName familyName -> {return std::string($1) + std::string($2);};
```

However, this makes development a bit more difficult because you probably want to write the parser first before generating any values, and most likely the types won't match. In that case, I recommend to ignore all values first:

```d
myChoice := ~number | ~(givenName familyName);
```

## Recursion := Recursion

It is a bit of a shame that recursion needs extra treatment. It would be nice if that wasn't necessary. Maybe something can be done if all rules are class members with defined return type or something...

Anyway, the first idea was to make the recursive rule mutable, first defining its type, letting it be referenced by other rules and then defining it, possibly depending on those other rules or itself. References are parsers with lambdae that capture an actual reference of the rule and when it changes, the lambdae use the new rule.

I found a bit ugly that references need some special syntax because they need some special treatment but I intended to fix that later.

First syntax looked like this:

```d
(R) := {std::string_view} -> {int}; // forward declaration
S := "(" @R ")"; // reference
R :> S; // definition
```

I did a big refactory where all lambdae captured referenced rules by reference and temporary (rvalue) rules by value:

```d
A := .;
B := A A; // A is captured by reference
C := "x" | "y"; // the literal parsers (which are not stored in a variable somewhere else) are captured by value
```

This is what std::forward is usually great at, except [it doesn't work with lambda captures](https://vittorioromeo.info/index/blog/capturing_perfectly_forwarded_objects_in_lambdas.html). So I used these wrappers, already unhappy how this inflates the code into very C++esque unintuitive complicatery. To be sure that I used references everywhere, I deleted the copy constructor of the parser class and only allowed move. This made it impossible to use std::function in the forward declaration of recursive rules, as std::functions must be copyable, so they cannot be assigned a lambda that captures uncopyable parsers. C++23 will have [move only functions](https://en.cppreference.com/w/cpp/utility/functional/move_only_function/move_only_function) which worked but I want to stay C++20 for now. I was thinking to throw() inside the copy constructor but I instead decided to screw the whole std::forwardery and copy around parsers like ints everywhere and use std::shared_ptrs instead of references. The code looks just so much better and I can retro justify it by saying it's safe because there won't be any dangling references. Also worked on the syntax a bit, now looks like this:

```d
expression' : {std::string} -> {int}; // forward declaration
bracedExpression := "(" expression' ")"; // reference
expression' => primary | bracedExpression; // definition
```

I like how the `prime'` thing is subtle but visible. I dislike how it messes up C++ syntax highlighting tho and I don't want to implement a proper custom one (which is impossible for github). Maybe I will switch to `@expression` or `expression@`...

Switched to `expression^` now that `^` is no longer used for actions (see below).

Working on the syntax again: https://chatgpt.com/share/6a724bfc-fd14-83eb-8824-322ead15e985

New syntax:
```d
%expression : {std::string} -> {int}; // forward declaration
bracedExpression := "(" expression ")"; // reference
%expression := primary | bracedExpression; // definition
myBoundAction := a b => {...};
myBoundPredicate := a b => {?...};
```

## Whitespace, the Final Frontier

I once heard about a whitespace operator in C++ (was probably a joke) and was thinking it'd be kinda cool to be able to write `a b` for `a*b` like in mathematics. Now that I am actually confronted with one, I must say that it has many annoying implications. Since a whitespace between two expressions in most parsing languages implies a sequence, we are very restricted with parentheses and letting symbols have different meenings as prefix or postfix. For example, the most intuitive syntax for macros (parameterizing rules) would be with appended parentheses, like this:

```d
list(X) := X ("," X)*
```

But then if we call that in a rule, it looks like this:

```d
identifierList := list(identifier)
```

And here we instead have a sequence of `list` and `(identifier)`:

```d
identifierList := list (identifier)
```

Uh oh, that is too similar. In [cpp-peglib](https://github.com/yhirose/cpp-peglib) this is solved by prioritizing macro calls. One wouldn't put a single identifier in parentheses if it's not a macro call. If there are more than one identifiers in there, a comma between them decides whether it's a macro call or a parenthesised sequence. It's ok I guess but it doesn't feel nice. Python would probably solve it by making `A(B)` a sequence and `A(B,)` a macro call but that just gives me eye cancer.

And that's not all. [OMeta/JS](https://github.com/alexwarth/ometa-js/blob/master/bs-ometa-compiler.txt) uses `A?` for optionals and `?(...)` for semantic predicates. And what is `A ? (f)`?

We can solve this by actually making `A(B)` a macro call and `A (B)` a sequence, and similarly making `A? (f)` a sequence of an optional `A` followed by `(f)` and `A ?(f)` an `A` that is accepted if `f` is true (except we will use {} but that's not the point). However, significant whitespace doesn't feel like a good idea somehow. No language I know has significant whitespace, it's probably a whole nother can of worms.

We could also solve it by just using some explicit sequence operator like `A, B`. But [PEG](https://en.wikipedia.org/wiki/Parsing_expression_grammar#Examples), [ANTLR](https://www.antlr.org/) and even the [Dragon Book](https://en.wikipedia.org/wiki/Compilers:_Principles,_Techniques,_and_Tools) use whitespaces for sequences. [EBNF](https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_form) uses commae... But I am actually also a friend of making my grammars reflect whitespaces (in the source they parse) explicitly like this:

```d
_ := (" " | "\n" | "\t")*
Addition := Number (_ "+" _ Number)*
```

instead of having identifiers and operators swallow all following whitespaces like in [the original PEG grammar](https://bford.info/pub/lang/peg.pdf). And how would `A, _, B` look compared to `A _ B`!? Ugly, that's how. So I will try to stick to whitespaces and find workarounds for all the problems they cause.

For now:

```d
list[X] := X ("," X)*
identifierList := list[identifier]

optional? ^?{return predicate;}
```

"Wait a minute!" I hear you think. "What happened to `[a-z]` ranges?" Let's be honest, they are not as useful in language design as they are when hacking together regular expressions for doing a quick search and replace operation across your codebase. You really only ever need them for specifying numbers and identifiers, maybe some unicode shenanigans. If the range syntax is slightly more elaborate than `[a-z]`, it doesn't really hurt. The `{'a'}..{'z'}` syntax can also express ranges for non text parsers, as long as the source elements can be compared.

## Grand Unified Theory of Actions and Predicates

It would be nice to unify the syntax for actions and predicates somehow. Actions genereate semantic values, predicates decide if parsing continues. The solution right now looks like this:

```d
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

```d
rule1 := A B {return fn();} C;
rule2 := A B -> {return fn($1, $2);} C;
rule3 := A B &{return fn();} C;
rule4 := A B -> &{return fn($1, $2);} C;
```

Also, I want to provide the ability to omit `return` and `;` if it's a single expression, so:

```d
rule1 := A B {fn()} C;
rule2 := A B -> {fn($1, $2)} C;
rule3 := A B &{fn()} C;
rule4 := A B -> &{fn($1, $2)} C;
```

Found a problem, this doesn't work:

```d
predicate := {fn()};
conditional := &predicate something
```

The `&` would confusingly not be an operator but actually part of the `&{}`, the above thing counterintuitively translates to `{fn()}` being an action and `&predicate` always passing.

Had two more ideas:

```d
rule1 := A B !{fn()} C;
rule2 := A B -> !{fn($1, $2)} C;
rule3 := A B ?{fn()} C;
rule4 := A B -> ?{fn($1, $2)} C;
```

If there is no `{...}` without any prefix and it's always `!` for actions or `?` for predicates (like in OMeta/JS), the situation is never ambiguous. But I don't like it. Actions should be without extra decorations.

Enough! The only way to prevent people from trying to rip apart the `{...}` and the `?` or `&` is to put the latter into the former. Most other parser MCs use `?` for semantic predicates, so will we. It can go in the beginning or the end but it's easier to parse if it's in the beginning, so `{?fn()}` it is!

Also, there's not really much point in making a difference between `{?...}` and `-> {?...}` as the latter will pipe everything through anyway (you can see in the comments of the first listing in this section that rule3 and rule4 return the same thing). So we will just omit the arrow here. Then we're also very inline with OMeta/JS.

Houston, we have another problem:

```d
predicate := {? checkA($)}
rule := A predicate
```

This doesn't work if checkA can't handle `ignore` as input argument. The first line creates a parser that doesn't feed anything (and hence does feed `ignore`) into the lambda that forms the predicate. In the second line, a new parser is created that does feed the semantic value of `A` into the lambda. Even if the predicate is never used without an argument (and hence `ignore` as an argument), the first line still wants to instantiate it for `ignore`.

I already tried making the `Predicate` class not inheriting from `Parser` but implicitly casting to one but this cast doesn't always happen. We can't call a `parse` method on it for instance.

`defer` to the rescue! I already only saw syntactically hideous solutions to this misery, when I clutched to the last straw: When we call the predicate lambda in the predicate parser, we run its argument (`ignore`) through a template that also requires the type of the source code as a template argument. This way, the thing doesn't get instantiated if the source type is not known, which only becomes known when the whole predicate parser is actually put into action without arguments.

In order to simplify the horrible wrapper overloads for predicates and actions and make them reusable for further wrappers instead of causing combinatoric explosion, I've now given wrappers >= and > operators that unwrap and forward. This means:

```d
0 myAction := {...}
myAction = Logger<0>(Rule<myAction>({...}))

"abc" -> myAction
"abc"__lit__ >= Logger<0>(Rule<myAction>({...}))

calls operator>=(parser, Logger<0>(Rule<myAction>({...})))

calls log<0>(operator>=(parser, Rule<myAction>({...})))

calls rule<myAction>(parser, {...})

which then calls operator>=(parser, {...}) which returns a parametricAction
```

So this unwraps, parametrizes the action and rewraps. Problem is that before I had operator>= for actions (-> in ometa) and operator> for both sequences and predicates (whitespace in ometa). However, we dont want unwrapping and rewrapping with sequences, only for parametrization of actions and predicates, but the wrapper operator> doesn't know and just operates on all of them. So predicates now also get >= (-> in ometa) for parametrization. This also means that we can have predicates in sequences that operate on empty input. It also means we only have to deal with one operator for unwrapping and the whole situation is more consistent. Today is a good day.

## Snippets

Most likely you want to puzzle strings together as the output. Just using std::strings and +ing them will cause a lot of data being copied around. Instead, we use trees of views that can be mixed and matched. In the end, we just iterate through the whole tree and output all the views. However, I wasn't able to find a syntactically sweet way to do this.

* `"abc"_S` for `ometa::ViewTree<std::string_view>("abc")` works but is a bit ugly and annoying.

* `"abc"S` would be less annoying but causes `warning: user-defined literal suffixes not starting with '_' are reserved;`. Well, actually, I can convert `S` to `_S` during transpiling... but it still looks shit somehow. Maybe there is a less ugly character.

* `'abc'` would have been nice since multi-character literals have implementation-defined behavior anyway and are thus discouraged and we can just use the syntax for our purpose. However, `'a'` should actually still be a `char` :/

* Creating assimilating `operator+`es like `std::string` is a bit nice.
```d
ViewTree + ViewTree -> ViewTree
ViewTree + const char* -> ViewTree
const char* + ViewTree -> ViewTree
```
`ometa::any`, `ometa::capture` and `ometa::range` all return `ViewTree`s and you will likely puzzle strings around those results, so you will almost never have to manually convert a string literal to a `ViewTree`. However, it's all fun and games until you get used to that and encounter a situation where it stays a string literal:
```
ViewTree("abc") + "def" + "ghi"; // works
"def" + "ghi"; // does not work
ometa::action([](auto value){return "abc";}); // also returns a const char*
```
so I think manually declaring a literal to be a `ViewTree` is better.

Using neither `"double quotes"` nor `'single quotes'` and for example `` `backticks` `` completely messes up standard C++ syntax highlighting again...

`"abc"_` is a last option but I somehow also don't like it.

I can also just convert all `"abc"` literals to `ViewTree`s but that also doesn't feel right.

I got it!: `'"abc"'`

I'm still not ultimately satisfied but I and people will have to learn to love it.

Now I have implemented \`this\` for view trees and used D for fake syntax highlighting.

## Putting Things into Context

My original thought was: Semantic values are for moving data from child nodes to parent nodes (all the things that the parser eventually spits out) and the context is there for moving data from parent nodes to child nodes (symbol table, line and column, etc.). So I have implemented nodes that let you modify the context. However, I have made the context a const reference, so the context modifier needs to pass on a new context (that can point to the old context). This way, it cannot happen that a node edits the context and that change is not reverted in case of backtracking. All clean and proper and functional and beautiful and Haskellesque. Instead of modifying context, data should be passed back up through values.

However, I have noticed that there was a problem.

Action nodes look like this:

```d
(rule -> action)
// under the hood, simplified:
{
	auto ruleValue = rule.parseOn(src, context);
	auto actionValue = action(ruleValue, context);
	return actionValue;
}
```

while predicates look like this:

```d
(rule predicate)
// under the hood, simplified:
{
	auto ruleValue = rule.parseOn(src, context);
	auto success = predicate(ruleValue, context);
	return success ? result : fail;
}
```

and the new context modifier looks like this:

```d
@contextModifier rule // wasn't sure about the syntax yet
// under the hood, simplified:
{
	auto newContext = contextModifier(src, parentContext);
	auto ruleValue = rule.parse(src, newContext);
	return ruleValue;
}
```

See it? There is no way for values to influence the context.

Then I was thinking, do we actually need contexts? Let's say we want to parse XML where the closing tag depends on the opening tag. This can still be done with values alone:

```d
block := openingTag:o block closingTag:c {? $o==$c}
// heavily simplified, the recursion and the identical tagging of nested blocks need to be taken care of
```

But I think it would definitely be much more complicated or maybe impossible to implement symbol tables like this.

My next idea was to join the values of earlier nodes in a sequence with the context, meaning:

```d
A B C
// under the hood, simplified:
{
	auto AValue = A.parseOn(src, parentContext);
	auto BContext = join(parentContext, AValue);
	auto BValue = B.parseOn(src, BContext); // actually src should be src + what A consumed
	auto CContext = join(BContext, BValue);
	auto CValue = C.parseOn(src, CContext);
	return CValue;
}
```

However, this also changes the type of the context which has to be considered when declaring recursive rules and later defining them. Complicated and annoying, I probably already lose half the user base because this has to be considered for the values.

Then I thought, maybe we can have a special magic operator that turns a value into context:

```d
A @ B // wasn't sure about the syntax yet
// hood:
{
	auto AValue = A.parseOn(src, parentContext);
	auto BContext = AValue;
	auto BValue = B.parseOn(src, BContext);
	return BValue;
}
```

Buuut can we have symbol tables now? Lettuce see. We want to be able to pasrse:
```d
a=1
b=2
c=a+b
```
and throw if we encounter an unknown variable.

```d
statement := identifier "=" term ("+" term)* "\n"
term := number | knownIdentifier
code := statement*
```

`knownIdentifier` and hence `term` and hence `statement` need all previous identifiers handed to them via context. Where do we put the magic operator? We need to squeeze it into the repetition of statement...

```d
code := statement @ statement @ statement ...
     := (statement @)* ?!
```

So this either needs to be done by default in repetition or there has to be an extra repetition, maybe `@*` and also `@+`? So then context becomes the parent context joined with a deque of all previous values?

It all feels awkward and also causes more type juggling.

Maybe it would be better to actually allow modifying the context. Then it can also have the same type across all rules. However, a mutable context does not mix well with backtracking. Imagine the following contrived language:

```d
a=1
b=2
c=3 <- ignore
d=4
```

which we parse with the following grammar with a map as context:

```d
assignment := identifier:i "=" number:n -> {context[$i] = $n} "\n"
commentMarker := " <- ignore\n"
comment := (!commentMarker .)* commentMarker
line := assignment | comment
code := line*
```

The problem is that the `assignment` rule mutates the context, then gets to the point where it expects a line break and then maybe fails and backtracks (as with line 3), then checking the comment branch. But the change to the context remains. This can be prevented by making a backup of the context before every rule invokation and reverting to it in case of backtracking. This can even be done automatically in the parser parent class:

```d
	auto parseOn(src, auto& ctx) const {
		auto backup = ctx.copy();
		auto result = parseFn(src, ctx);
		if(! result.has_value()){
			ctx = backup;
		}
		return result;
	}
```

However, this would require a metric ton of copying (0.984207 imperial tons). Another option would be to leave this to the discipline of the programmer, which feels very C++ (in a bad way) but most languages are constructed in a way that does not make much backtracking necessary, so this is probably likely not a very problematic issue maybe. Then again, having a parser that allows for infinite backtracking but then discouraging it has a bit C preprocessor vibes (in a bad way).

Or maybe we create yet another class which accumulates changes to the context and either applies them or ignores them, but this would require iterating through the whole context log everytime we require its current actual state. On the other hand, would it be much different with immutable contexts? It wouldn't. The idea of immutable context pointing to a chain of parent contexts is essentially also a linked list with all its O(n) indexing performance.

Here's the new idea after a night of "sleep": A context is a kind of multimap where an entry also knows its insertion index (i.e. the size of the multimap before the entry was inserted). The key is for instance the symbol name. Backtracking now works like this:

```d
	auto parseOn(src, auto& ctx) const {
		size_t contextSizeWhenThisNodeWasInvoked = ctx.size();
		auto result = parseFn(src, ctx);
		if(! result.has_value()){
			ctx.eraseAllElementsWithIndexAbove(contextSizeWhenThisNodeWasInvoked);
		}
		return result;
	}
```

All children of this node may only add new entries to the context (although an entry can also instruct to regard this symbol as deleted). When reading in the context, only the latest entry of a symbol is considered.

What is the best way to implement this? `multimap` or `unordered_multimap` are not really usable since `find()` returns a random element with a key, not the last one and also `erase()` erases all elements with a key. We can use `unordered_map<Key, stack<Entry>>` and store the insertion order in an extra `stack<Key>`. Now, if we want to backtrack, we keep popping keys from the order stack and from the corresponding symbol stack. This is it! This feels good! Assuming that entries are usually not overwritten and backtracking will usually not remove more than one element, this should be very efficient.

# Done

Getting back to this after 2 years... the syntax looks very cluttered. I think it needs some improvements:
* I think I never actually use the captured whitespace so _ should return ignore already and I should never write ~_, I can also have some extra ws for captured whitespace -> implemented, beautiful.
* '"blah"' is just too much, it should be \`blah\`. I can write a custom highlighter for vscode but dont know what todo about github. Maybe \$"blah" would be a nice option? semantic values are \$0 \$1 etc, string puzzle values can be \$"sheesh"... -> now I have implemented \`this\` for view trees and used D for fake syntax highlighting.
* most string literals are ignored, maybe we can use 'blah' for ignored and "blah" for significant -> I have now implemented this. It looks much cleaner. Problem is that in Python as well as in PEG, '...' and "..." are identical and ~"..." was more explicit. Lets see how the giant userbase will react, I mean we can always roll it back, right?
* right now EVERYTHING backs up the context, do we maybe only need it in actions and predicates? -> no, we need it everywhere. Everything that can fail itself needs to backup the context and restore it from all the changes its subparsers might have done.

### Parser Refactory

I wanted to rewrite the Parser class. parseFn should be private, parsers should get a name tag:
Parser<Tag name, typename F>
so they can easily be identified in compiler spam. Then parser parameters in whatever external functions should use ParserLike concept so they can handle Parser-derived classes without slicing if the parser carries some extra data around. Maybe slicing is also ok, dunno.
Anyway. On that note I wanted to make the parseFn private. I also wanted to take some functionality outside of the class and make it a bit smaller. as() can be an external function. operator=() should go away, the parseFn should be immutable and Parser-pointers should have the child swappable. Maybe we can even somehow unify parse and parseOn into a single function and then a parser is something that only has a parse() method. But then if theres only the parse() method, we can even use operator() for that and have a Parser just be a function/functor...

https://chatgpt.com/share/6a64a144-ff80-83ed-8cae-151c4e7df1fe

new approach:
Parser is a wrapper around parseFn to provide convenience functionality. parseFn is immutable-ish (only has a getter) but we need it to be mutable for recursion. We can't let pre-declared parser pointers have a swappable child since we then need to know the super nested complicated Parser type of the child in advance. We rather just type-erasingly specify the type of its parseFn and then rip it out. So the parseFn must be out-rippable... oops...
On that note, check if the parseFn is actually out-rippable, given all the context-specific decoration in parseOn... I think the context stuff is alright since the new parser does that as well. but the logging will be replaced. need to investigate.

sheesh I should definitely move to a parser just being a std::function. And all these generator functions should return Functors. In fact, the generators can be classes with operator() as the parser.
BUUUUT I could lose the ability to track down every parse call...
except those can then be done in an actual debugger.
benefit is actually usable compiler output.
I should start by implementing recursion first, that is most likely to byte me. maybe custom parser combined rules can also be classes with a sensible name...

I've just noticed that context backup cant only happen in actions and predicates. Only actions and predicates can change the context but anything that can fail needs to backup the context and restore the changes from its subparsers. Hence a Parser wrapper is probably a good idea after all.

One major argument for just using functions was that type erasure on those is trivial, which I need for pre-declaring and later assigning recursive parsers. This is now solved via Parser objects only being wrapper shells and by convention allowing to rip the parseFn out of them then recursion works like this:

```
shell := ParserShell with stub parseFn of type: std::function...
stuff := a | b | shell // some combinations referencing shell
shell.parseFn = (c | d | stuff).parseFn // some combinations referencing stuff that references shell
```

This rips out the parseFn from whatever is assigned to it, which feels super ugly. I'd rather save that as a child Parser in the shell but didn't know how without having a Parser base class with virtual destructor and all that virtualization overhead.

Here's how:

```
struct Shell{
	shared_ptr<Parser<std::function...>> child;
}
shell := Shell<std::function...>{};
stuff := a | b | shell // some combinations referencing shell
shell.child = make_shared<SpecialParser<F>>(...)
```

shared_ptr can do type erasure without virtual destructor. We could even do shared_ptr<void>.

Maybe even sexier:
```cpp
template<class B, class D, class... Args>
std::unique_ptr<B, void(*)(B*)>
make_erased_unique(Args&&... args)
{
    return {
        new D(std::forward<Args>(args)...),
        [](B* p) {delete static_cast<D*>(p);}
    };
}
```

shared_ptr probably safer and less to worry about. Also, we don't need all references to it to be any special anymore. Parser handles the pointing already internally and can be value-copied around and all occurrences update when child gets assinged.

expression : {int} => {int}
stuff := a | b | expression
expression => ...


What I was just now trying was this:
```cpp
template <typename TSource, typename TValue, typename TContext>
class MutableParser{

	std::unique_ptr<Parser<TSource, TValue, TContext>> child;

	MutableParser():Parser{
		[this](View<TSource> src, TContext& ctx) -> MaybeMatch<TValue, TSource> {
			if(this->child){	
				return child->parseOn(src, ctx);
			}
			else{
				throw std::runtime_error("mutable parser not defined");
			}
		}
	}{}

	template<DerivedFromParser P>
	void setChild(const P& newChild){
		child = make_type_erased_unique<Parser<TSource, TValue, TContext>, P>(newChild);
	}	
};
```
I thought I'd rewrite the Parser<F> to Parser<TSource, TValue, TContext> instead but then it's impossibru to store lambdas in it like I do. If I rewrite with a base class Parser<TSource, TValue, TContext> with a fix parse() member that throws and has to be overwritten with template inheritance, I lose the code reuse possibility I use now because base parse() cant call child methods without virtualiticity.

Now rewritten the recursion part. I dislike how it uses shared_ptr as well as std::function. We can already do type erasure with shared_ptr, there should be a way to avoid std::function. But fine for now. Here is the minimum example:

```cpp
shared_ptr<function<int(int)>> wrap =
    make_shared<function<int(int)>>(
        [](int)->int{
            throw runtime_error("undefined");
        }
    );
    
auto wrap2 = wrap;

*wrap = [](int x){return x+1;};

cout << (*wrap2)(5);
```

If we want to really squeeze, we can invert the update direction. Right now, a shared_ptr is used so every occurrence of the yet-undefined parser will point to a single instance and once that is updated, they all point to the correct parser. Instead we could keep track of instances and once the parser is defined, it updates all the instances, so there will be one less indirection on the hot path. But in the end we are using recursive descent, we shouldn't worry about performance too much.

### Compiler Error Readability

I was thinking I should move away from lambdas and use functors in the generators. I've tried it with choice. Instead of a lambda which appears as `(lambda at ...)`, it appears as `ometa::ChoiceFn<ometa::Parser<(lambda at update/include/sequence.h:11:17)>, ometa::Parser<(lambda at update/include/action.h:50:17)>>::operator()<ometa::View<std::basic_string_view<char>>, ometa::Empty>` I think this is worse tho as this will nest to oblivion and lambda actually prevents this nicely.

Furthermore, I've made a class Choice that inherits from Parser and does nothing else, only to give Parser a name. However, often we will still see `ometa::Parser<ometa::ChoiceFn<` because it refers to the passOn method which is defined in Parser, not in Choice.

So I will now try to give Parser a tag.

actions, predicates and recursives must not be wrapped by the rule wrapper, otherwise their mechanism wont work later. -> solved, we can now take things out of the rule wrapper.

I gave parsers a tag with a string literal, it increased compile time from 6.3 to 8.6 seconds. Now each Parser gets a custom class as tag. It looks even better in the compile error and compiles faster.

### Selective logging

idea:
We can call ometa-cpp with different levels of verbosity: (nothing), -v, -vv, -vvv

By default -v does nothing, -vv prints every rule invocation and -vvv prints every parseOn invocation.

But we can tweak that in code! Numbers are still free, so we can use them to change debug levels of parsers.

```d
// set the debug log threshold of myRule to 0:
0 myRule := a | b | c;

// set the debug log threshold of myRule and every rule call inside to 0:
00 myRule := a | b | c;

// set the debug log threshold of myRule and every parseOn call inside to 0:
000 myRule := a | b | c;

// set the debug log threshold of this occurrence of b to 0:
myRule := a | 0b | c
myRule := a | 0 b | c

// we can also set all that to 1, 2 or 3
0 myRule := 1 a | 2 b | 3 c

// and also mix levels: myRule is 0, subrules are 1, parseOn calls are 2
012 myRule := a | b | c;

// omitting digits means inherit from outside.

```
I think this should be the highest operator precedence.
Inner overrides outer.

recursion, actions and predicates also need to be able to deal with rule(log(standalone)) :/ maybe I find a better way -> done, works for now, ugly

### Misc

* the rewrapping of logging rules of predicates and actions is unacceptably ugly -> done, still rewrapping but no longer as ugly
