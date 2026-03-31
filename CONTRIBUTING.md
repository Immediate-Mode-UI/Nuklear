# Contributing

## Reviewers guide

When reviewing pull request there are common things a reviewer should keep
in mind.

Reviewing changes to `src/*` and `nuklear.h`:

* Ensure C89 compatibility.
* The code should work for several backends to an acceptable degree.
* Check no other parts of `nuklear.h` are related to the PR and thus nothing is missing.
* Recommend simple optimizations.
  * Pass small structs by value instead of by pointer.
  * Use local buffers over heap allocation when possible.
* Check that the coding style is consistent with code around it.
  * Variable/function name casing.
  * Indentation.
  * Curly bracket (`{}`) placement.
* Ensure that the contributor has bumped the appropriate version in
  [clib.json](https://github.com/Immediate-Mode-UI/Nuklear/blob/master/clib.json).
* Have at least one other person review the changes before merging.

Reviewing changes to `demo/*`, `example/*` and other files in the repo:

* Focus on getting working code merged.
  * We want to make it easy for people to get started with Nuklear, and any
    `demo` and `example` improvements helps in this regard.
* Use of newer C features, or even other languages is not discouraged.
  * If another language is used, ensure that the build process is easy to figure out.
* Messy or less efficient code can be merged so long as these outliers are pointed out
  and easy to find.
* Version shouldn't be bumped for these changes.
* Changes that improves code to be more inline with `nuklear.h` are ofc always welcome.

