# Triplet Structure Runtime

This folder contains a triplet structure runtime implementation. The goal of
the runtime is to take a Triplet Structure (see ``../ts_lib.py``), parse the
"operational" nodes (eg. rules and tactics), then apply the rules so that some
goal is achieved (eg. getting rid of TOP nodes).

At a high level:
- `runtime.py` operates as the main data structure which keeps track of the
  information needed and produced during the execution.
- `solver.py` has methods that allow for existential queries on the
  corresponding Structure. For example, you might look for nodes 1, 2 such that
  facts (1, 2, "/:Mapper:TOP") and (2, 1, 2) both hold in the structure.
- `pattern.py` is essentially a higher-level interface to the Solver. Each
  `Pattern` instance corresponds to a single pattern/existential query.
- `production_rule.py` can be thought of as an _even higher_ level interface to
  the `Patterns`. Each `ProductionRule` corresponds to one ``/RULE` node in the
  Structure. ProductionRules can have relatively complicated mapping
  strategies, eg. "find an assignment satisfying ... but not ..."
- `assignment.py` describes a satisfying assignment to a `ProductionRule`.
  `Assignment`s are an intermediate step between matching a pattern and then
  constructing the actual delta corresponding to the match/rule.
- `delta.py` describes `TSDeltas`, which describe a modification (add/remove
  nodes/facts) to the structure. They are produced by `Assignments` which are
  produced by `ProductionRules`.
- `tactic.py` describes `Tactic`s, which are essentially heuristics that
  control the application of rules (and other tactics).
- `interactive.py` is an interactive REPL for manually/semi-automatically
  applying rules, tactics, etc. to the structure.
- `shadow_input.py` is used by `interactive.py` to allow the user to record,
  save, and replay their commands.
- `utils.py` contains helper methods for the rest of the runtime. Most useful
  is the Translator, which cleans up some common operations with dictionaries.
