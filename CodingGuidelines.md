FastArduino Coding Guidelines
=============================

Preamble
--------
Why do we need guidelines?
Where do these guidelines apply?

Naming
------
Throughout FastArduino library code, the following naming conventions apply:

1. Namespaces: always writtent in `lower_case` (possibly including underscores to separate different words); one exception to this rule is when a namespace is defined to be used like an extensible `enum class` and thus contains only `static const` fields, all of the same type, e.g. `board::PWMPin`.
2. Template arguments: always written in `UPPER_CASE`, often ending with an underscore (`UPPER_CASE_`).
3. Macros: always written in `UPPER_CASE` for "public" macros (part of FastArduino official API) or `UPPER_CASE_` for internal macros (not part of the API)
4. struct, enum, union, class types: always written as `CamelCase`, possibly with a trailing underscore for internal (not public) types.
5. enum values: always `UPPER_CASE`
6. Constant values (e.g. `static const...`): written in `UPPER_CASE` or `UPPER_CASE_`.
7. functions, methods: always written in `lower_case`; uppercase letters may be used for official measurement units though (e.g. `voltage_mV()`); when one function exists in 2 flavours, one `synchronized` and one not `synchronized`, then the not synchronized flavour bears the same name as the synchronized function, with a trailing undercore added, e.g. `Queue::push()` and `Queue::push_()`.
8. `private` fields of a class: always written as `lower_case_` with a trailing underscore; uppercase letters may be used for official measurement units though.
9. Local variables are always written as `lower_case`; uppercase letters may be used for official measurement units though.
10. `static constexpr` functions: not yet defined; hesitating between `UPPER_CASE` and `lower_case`, currently fastArduino has methods with both conventions.
11. Types alias with `using` should always be `UPPER_CASE`.

Currently, FastArduino has no automatic check of these guidelines, hence control must be eprformed manuaaly before merging Pull Requests.

Coding Style
------------
All other conventions for coding styles are directly defined within project's `.clang-format` file rules.

Currently FastArduino uses clang-format 4.0 because this is the most supported version across Linux distributions; however, its current setting are not fully satisfactory and are not up to my expectations for FastArduino, in particulare, I am not happy with the results obtained from the current settings on the following topics:
- spaces used instead of tabs when aligning `\` at end of lines (used for macros)
- spaces used in addition to tabs when aligning 2 lines (e.g. template or function arguments)
- line breaking of ternary operator after both `?` and `:` where I would like break only after `:`
- short for loops on2 lines instead of only one
- template arguments not broken at one per line if I want so
- buggy value alignment in assignments
- no line break before `{` in `extern "C" {` construct (new setting in clang-format 5.0)
- empty function not properly written as one line e.g. `void f() {}` (that setting exists but does not work properly in many situations)

