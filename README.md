# Sifter
This repository contains code implementing the prototype analogy-making program
Sifter, as described in our 'Onward!' 2020 paper "Analogy-Making as a Core
Primitive in the Software Engineering Toolbox."

Sifter can make _analogies_ about programs, e.g., identifying data structures
and methods which play corresponding roles in two different programs. Sifter
can also _complete_ analogies, which allows it to automatically learn and
generalize source code transformations from a small number of examples.

#### Dependencies
You must install [Bazel](https://bazel.build) as well as set up
[bazel_python](https://github.com/95616ARG/bazel_python) with Python 3.7.4.

#### Running Tests, Examples
You should then be able to run
```bash
bazel test //... && bazel run coverage_report
```
to get a coverage report in `htmlcov/index.html`.

To run the examples:
```bash
bazel run examples/letter_analogies:letter_analogy
bazel run examples/turing_machine:turing_machine
bazel run examples/program_analysis:program_understanding
bazel run examples/program_analysis:api_migration
bazel run examples/program_analysis:transform_learning
```
For the `program_analysis` ones, you can view the result by visiting
`http://localhost:8001` in your browser once prompted.

#### Goals, Status, and Future Work
This repository accompanies our paper in
[Onward! 2020](https://2020.splashcon.org/track/splash-2020-Onward-papers?#the-character-of-onward),
with the goal of encouraging interest and future work in automated
analogy-making for software engineering.  Ultimately, we see analogy-making as
involving three main processes:
1. _Raw Perception_, such as reading files (code, documentation) from the
   filesystem and into the triplet structure workspace.

   **Status:** The `LazyStructure` and `LazyTextDocument` classes in
   [examples/program_analysis/lazy_structure.py](examples/program_analysis/lazy_structure.py)
   read a file from the filesystem and add a representation of the file's
   contents to the triplet structure workspace. These interfaces are 'lazy' in
   that they allow selecting only certain parts of a file to be included in the
   workspace. Currently we either specify ahead-of-time which portions of the
   file to include in the workspace or just add the entire file contents.

   **Future Work:** We envision the laziness being used to initially focus the
   analogy-making on a small subset of the relevant code, then expanding to
   larger portions as the analogy solidifies.
2. _Semantic Rewriting_, such as grouping individual characters into a single
   token, inlining a function, and annotating invariants found via program
   analysis.

   **Status:** Currently, the default `LazyTextDocument` interface performs a
   light-weight tokenization pass that groups characters separated by spaces
   and special characters (such as `+`) before encoding the document into the
   workspace. Extra semantic information, such as program invariants, are
   specified ahead-of-time by either directly editing the workspace or using
   the `AnnotateFact` method of `LazyTextDocument`. There is an example of
   annotating program invariants in
   [examples/program_analysis/transform_learning.py](examples/program_analysis/transform_learning.py).

   **Future Work:** We would like to directly connect Sifter to program
   analysis tools so analysis results can be automatically imported into the
   triplet structure workspace instead of needing to be manually annotated.  We
   would like to incorporate rewrite rules that express semantic equivalence,
   such as inlining, syntax de-sugaring, and grouping. Finally, we would like
   to develop heuristics that can determine when to apply a program analyzer or
   semantics-preserving rewrite rule. Such a heuristic would need to operate in
   tandem with and be guided by the abstraction/anti-unification process.
3. _Abstraction/Anti-Unification_, where we pair up corresponding objects in
   the workspace to form an analogy.

   **Status:** We have fairly-complete rules and heuristics for this that are
   described in more detail in [AnalogyUtils.md](AnalogyUtils.md). These rules
   roughly implement a syntactic anti-unification on unrooted, labelled graphs.

   **Future Work:** Our existing abstraction rules work well, but depend on the
   semantic rewriting to have already exposed most of the correspondences in
   the syntactic representation of the workspace itself. So the primary future
   work for this process is to provide feedback to the semantic rewriting
   engine. For example, given programs `if x: y += 1` and `z += a ? 1 : 0;` we
   may not be able to initially find a good syntactic abstraction, but we want
   the abstraction process to be able to give feedback to the semantic
   rewriting process to, e.g., desugar the ternary `?:` operator into the
   corresponding `if` statement, which we can then find a syntactic
   correspondance with. Beyond such better heuristics, we would also like to
   support higher-order 'slips,' i.e., analogies where the types themselves are
   abstracted.

In addition to these three processes, there are two other main directions for
future work:
* The runtime does not currently support modifying rules with other rules; in
  fact, rule-related nodes are removed from the structure completely after an
  initial rule-parsing pass. In the future we may decide to change this to
  assist with meta-learning.
* We are still working on a good visualization module for triplet structures.
  There is a rudimentary CLI interface
  ([runtime/interactive.py](runtime/interactive.py)) as well as an interface
  specifically for source code analogies
  ([examples/program_analysis/ui](examples/program_analysis/ui)), however we
  plan to introduce a more general-purpose interface in the near future.

#### High-level File Overview
- [ts_lib.py](ts_lib.py): the core library, defines the triplet structure data
  structure and some embedded-DSL-style helpers for describing triplet
  structures.
- [ts_utils.py](ts_utils.py): a set of macros which make working with the
  library (especially expressing rules) easier.
- [tactic_utils.py](tactic_utils.py): a set of macros which make writing
  tactics ("controllers for applying the rules") easier.
- [mapper.py](mapper.py): rules which can be added to any triplet structure to
  enable making abstract analogies (i.e., identifying isometries in
  sub-structures).
- [analogy_utils.py](analogy_utils.py): a Python interface and tactics to
  building analogies in triplet structures that have the rules from
  [mapper.py](mapper.py) added to them.
- [ts_cpp/](ts_cpp/): C++ implementation of the core triplet structure data
  structure, as well as a backtracking triplet constraint solver.
- [runtime/](runtime/): a runtime to parse and execute rules on
  TripletStructures.
- [examples/](examples/): examples of using triplet structures to solve
  analogies.

#### Quickstart with the Code
Please see [TSLang.md](TSLang.md), which documents how to write code using the
Triplet Structure language.

#### Programming Style
We see much of the code in this repository as defining a domain-specific
language [TSLang](TSLang.md) embedded in Python. We then write, within
`TSLang`, the core analogy-making code for Sifter.

To highlight the difference between "plumbing" code implementing the `TSLang`
language and runtime vs. the actual analogy-making code written in `TSLang`, we
use a distinct coding convention for each type of code:
1. Code implementing `TSLang`, specifically `ts_lib.py` and `runtime/*`, is
   written using Google-style Python naming (e.g., `snake_case` for methods and
   `PascalCase` for classes).
2. Code written in `TSLang` (everything else) is written using `PascalCase` for
   method names.

This naming convention is enforced intra-file by `.pylintrc`.

#### Citing
```
@inproceedings{sifter:onward20,
  author    = {Matthew Sotoudeh and
               Aditya V. Thakur},
  title     = {Analogy-Making as a Core Primitive in the Software Engineering Toolbox},
  booktitle = {Proceedings of the 2020 {ACM} {SIGPLAN} International Symposium on
               New Ideas, New Paradigms, and Reflections on Programming and Software,
               Onward! 2020, November 18-20, Virtual, USA},
  publisher = {{ACM}},
  year      = {2020},
  url       = {https://doi.org/10.1145/3426428.3426918},
  doi       = {10.1145/3426428.3426918},
}
```

#### People
- [Matthew Sotoudeh](https://masot.net/): email
  [masotoudeh@ucdavis.edu](mailto:masotoudeh@ucdavis.edu).
- [Aditya Thakur](https://thakur.cs.ucdavis.edu/): email
  [avthakur@ucdavis.edu](mailto:avthakur@ucdavis.edu).

#### License
Licensed under the [AGPLv3](LICENSE).
