# TSLang
The goal of this file is to document how to begin writing Sifter code. It
is specifically directed towards understanding the high-level '`TSLang`'
interface, which is used to write the analogy-making rules (`mapper.py`) as
well as the application demos (`examples/...`).

### High-Level View
At the highest level, `TSLang` programs are programs that define and operate on
a particular type of data structure, a _triplet structure._ Most `TSLang`
programs are separated into two stages:
1. An initial triplet structure is defined with the data of interest, then
2. The initial structure is then iteratively modified by appling pre-defined
   _update rules._

`TSLang` provides syntactic sugar to help:
1. Define triplet structures (`ts_lib`),
2. Define rules operating on triplet structures (`ts_lib`, `ts_utils`), and
3. Write _tactics_ determining which rules to apply in what order (`runtime`
   and `tactic_utils`).

We will address each of these in turn.

### (0) What are Triplet Structures?
A triplet structure is a particular type of data structure suited to
representing knowledge about the world.

At its core, a triplet structure consists of exactly two things:
1. A list of _nodes_ and
2. A list of _triplet facts_, which are 3-tuples of nodes.

To give some meaning to the structure, we adopt the convention that a triplet
fact `(A, B, C)` should be interpreted like so:
1. `A` is represents a particular logical fact,
2. `B` represents an instance of something,
3. `C` represents the _type_, or role `B` plays in the fact.

For example, we might want to assert that `Homer` is the `Father` and `Marge`
is the `Mother` of `Lisa` with the facts:
```
(FamilyFact1, Homer, Father),
(FamilyFact1, Marge, Mother),
(FamilyFact1, Lisa, Daughter)
```
We think of `FamilyFact1` as the unified 'thought' or 'fact node' representing
the fact that Homer, Marge, and Lisa together make an instance of the "Family"
type, with the given roles.

For more discussion of triplet structures and how to represent logical
information with them, please see our Onward paper.

### (1) Representing Triplet Structures with `TSLang`
A triplet structure is represented by an instance of the `TripletStructure` class.
Most programs will only have one instance of `TripletStructure`, which we will
conventionally name `ts`:
```python
from ts_lib import TripletStructure
ts = TripletStructure()
```
Nodes in `TSLang` each have a unique, string name. With few exceptions, all
names should start with `/:`.  Nodes are referenced by indexing notation, and
are automatically created upon reference if they do not yet exist:
```python
# Creates nodes '/:Homer' and '/:Marge'
ts["/:Homer"], ts["/:Marge"]
```
To add a fact `(A, B, C)` we use the notation `ts[A].map({ts[B]: ts[C]})`, like
so:
```python
# Adds fact ('/:FamilyFact1', '/:Homer', '/:Father')
ts["/:FamilyFact1"].map({ts["/:Homer"]: ts["/:Father"]})
```
Multiple facts with the same fact node can be expressed naturally as well:
```python
# Adds 3 facts.
ts["/:FamilyFact1"].map({
    ts["/:Homer"]: ts["/:Father"],
    ts["/:Marge"]: ts["/:Mother"],
    ts["/:Lisa"]: ts["/:Daughter"],
})
```
To prevent name collisions and to better enable the usage of macros to
automatically manipulate the structure, we have support for _scopes_. A scope
is simply a prefix of a node name, up to (but not including) a `:`. Intuitively
we can think of node names as paths, with `:` delimiting directory boundaries,
and scopes playing the role of directories. For example, we might want to place
`Father`, `Mother`, and `Daughter` all in the `/:Family` scope, and `Homer`,
`Marge`, and `Lisa` in the `Simpsons` scope:
```python
ts["/:FamilyFact1"].map({
    ts["/:Simpsons:Homer"]: ts["/:Family:Father"],
    ts["/:Simpsons:Marge"]: ts["/:Family:Mother"],
    ts["/:Simpsons:Lisa"]: ts["/:Family:Daughter"],
})
```
In fact, `TSLang` has first-class support for scopes. By default, `ts[...]`
always indexes relative to its _current scope_, which can be changed using
`with ts.scope(...):`. The above is equivalent, for example, to:
equivalent to:
```python
# family[":..."] is equivalent to ts["/:Family:..."]
family = ts.scope("/:Family")
with ts.scope("/:Simpsons"):
    # In this block, ts[":..."] is automatically prepended with "/:Simpsons"
    # while ts["/:..."] is taken as an absolute path.
    ts["/:FamilyFact1"].map({
        ts[":Homer"]: family[":Father"],
        ts[":Marge"]: family[":Mother"],
        ts[":Lisa"]: family[":Daughter"],
    })
# Outside the with block, ts[":..."] again refers to ts["/:..."].
```
Scopes are generally used to group logically-related nodes together, especially
nodes which should be treated in a particular way by some macro (as we will see
later in this document).

### (2) Writing Update Rules to Operate on Triplet Structures
An _update rule_ is a program that searches for a pattern in the triplet
structure and then makes some modification to the structure according to that
pattern. With `TSLang`, we _define update rules within the structure itself._
We accomplish this by adding the pattern to search for as part of the
structure, and adding extra facts which annotate which parts of the pattern
need to be searched for or inserted/removed once an assignment is found.

The below example demonstrates this with a rule expressing transitivity of the
'greater than' relation:
```python
with ts.scope(":TransitivityRule"):
    # First we describe the pattern we want to search for:
    ts[":AGreaterThanB"].map({
        ts[":A"]: ts["/:GreaterPair:Greater"],
        ts[":B"]: ts["/:GreaterPair:Lesser"],
    })
    ts[":BGreaterThanC"].map({
        ts[":B"]: ts["/:GreaterPair:Greater"],
        ts[":C"]: ts["/:GreaterPair:Lesser"],
    })
    # Then what we want to insert if that pattern is found:
    ts[":AGreaterThanC"].map({
        ts[":A"]: ts["/:GreaterPair:Greater"],
        ts[":C"]: ts["/:GreaterPair:Lesser"],
    })
    # Finally, we encode the details of the rule:
    ts[":RuleFact"].map({
        # /:TransitivityRule:_ will represent the rule as a whole.
        ts[":_"]: ts["/RULE"],
        # The facts implying A>B, B>C should already exist in the structure
        # before we apply this rule, hence we 'must map' them.
        ts[":AGreaterThanB"]: ts["/MUST_MAP"],
        ts[":BGreaterThanC"]: ts["/MUST_MAP"],
        ts[":A"]: ts["/MUST_MAP"],
        ts[":B"]: ts["/MUST_MAP"],
        ts[":C"]: ts["/MUST_MAP"],
        # If they are found, we then 'insert' the A>C node and associated
        # facts.
        ts[":AGreaterThanC"]: ts["/INSERT"],
    })
```
Note that the nodes `/RULE`, `/MUST_MAP`, and `/INSERT` do not start with `/:`.
This indicates they are special nodes, which will be explicitly interpreted by
the runtime. Also note that the nodes `/:GreaterPair:Greater` and
`/:GreaterPair:Lesser` are not mentioned in the `:RuleFact`, which means they
will be treated as constants in the corresponding pattern.

Note also that, in addition to `/MUST_MAP`, other quantifications are possible:
`NO_MAP(#)` and `TRY_MAP`. In general, potential rule applications are
discovered in three passes:
1. First, we search for assignments to the constraints involving only the
   `/MUST_MAP` nodes.
2. For each of those assignments, we check to ensure that they can*NOT* be
   extended to a satisfying assignment to the `NO_MAP1`, `NO_MAP2`, ...
   constraints. We throw away any assignment which can be extended to also
   satisfy the `NO_MAP` constraints.
3. For any remaining assignments, we attempt to extend the assignment to also
   satisfy constraints involving the `TRY_MAP` nodes, if possible (otherwise
   the original assignment is used).
Similarly, in addition to `/INSERT` there are also other actions possible:
1. `/REMOVE` removes the node and _all_ associated facts.
2. `/SUBTRACT` removes only those facts explicitly mentioned by the rule (the
   node is removed if there are then no remaining facts).

Finally, note that by default we assume all of the `/MUST_MAP` nodes must have
_unique_ assignments. This can be explicitly weakened if desired to allow
pattern matches to assign the same node to two different `/MUST_MAP` variables.

### (3) Macros for Writing Update Rules
The above rule-declaration syntax can become tedious and error-prone if done by
hand. To assist in this, two macros are provided in `ts_utils.py` which
significantly improve the experience.

#### `RegisterRule`
The first, `RegisterRule`, is the most flexible. It works by putting nodes with
the same role in the rule (e.g., `/INSERT` or `/MUST_MAP`) in the same scope.
The rule from the previous example could be rewritten:
```python
with ts.scope(":TransitivityRule"):
    with ts.scope(":MustMap") as existing:
        ts[":AGreaterThanB"].map({
            ts[":A"]: ts["/:GreaterPair:Greater"],
            ts[":B"]: ts["/:GreaterPair:Lesser"],
        })
        ts[":BGreaterThanC"].map({
            ts[":B"]: ts["/:GreaterPair:Greater"],
            ts[":C"]: ts["/:GreaterPair:Lesser"],
        })
    with ts.scope(":Insert"):
        ts[":AGreaterThanC"].map({
            existing[":A"]: ts["/:GreaterPair:Greater"],
            existing[":C"]: ts["/:GreaterPair:Lesser"],
        })
    RegisterRule(ts)
```
Notice that we need to explicitly refer to `existing[":A"]` in the second
scope, as they are no longer all in the same scope.

#### `RegisterPrototype`
The second useful macro, `RegisterPrototype`, is useful when both:
1. You only need `/MUST_MAP` and `/INSERT` nodes, and
2. You want to define multiple rules which involve the same patterns.

An equivalent rule to the above is shown below:
```python
with ts.scope(":TransitivityRule"):
    ts[":AGreaterThanB"].map({
        ts[":A"]: ts["/:GreaterPair:Greater"],
        ts[":B"]: ts["/:GreaterPair:Lesser"],
    })
    ts[":BGreaterThanC"].map({
        ts[":B"]: ts["/:GreaterPair:Greater"],
        ts[":C"]: ts["/:GreaterPair:Lesser"],
    })
    ts[":AGreaterThanC"].map({
        existing[":A"]: ts["/:GreaterPair:Greater"],
        existing[":C"]: ts["/:GreaterPair:Lesser"],
    })
    RegisterPrototype(ts, dict({
        ":_": {ts["/INSERT"]: [ts[":AGreaterThanC"]]},
    }))
```
In general, the second argument can define arbitrarily many rules using the
exact same set of nodes. Each rule lists some subset of nodes which should be
inserted (alternatively, mapped).  The remaining nodes in the scope are assumed
to be mapped (alternatively, inserted). This feature is used extensively in
`mapper.py`, as all of the mapper rules are essentially just different
'shadings' of the same core pattern (see our Onward paper for more details).

### (2,3b) Update Rule Gotchas: Inserting Only a Triplet Fact
While our rule-declaration syntax is usually quite natural, there is one
particular case where care needs to be taken. Namely, when the rule needs to
operate on individual _facts_ about _existing nodes_. For example, suppose you
want to search for nodes `A`, `B` with a fact `(A, A, B)`, and insert a new
fact `(B, B, A)`. you might try to do the following:
```python
with ts.scope(":BadRule"):
    with ts.scope(":MustMap") as exist:
        ts[":A"].map({ts[":A"]: ts[":B"]})
    with ts.scope(":Insert"):
        exist[":B"].map({exist[":B"]: exist[":A"]})
    RegisterRule(ts)
```
The problem is that the `/:BadRule:Insert` scope is actually empty --- the line
there only refers to nodes in the `/:BadRule:MustMap` scope. Thus the
underlying rule created will only have `/MUST_MAP` nodes, not `/INSERT`, and so
the rule is effectively a no-op. To get around this, we need to ensure that at
least one of the nodes of each fact we want to insert actually belongs to the
`:Insert` scope:
```python
with ts.scope(":BadRule"):
    with ts.scope(":MustMap") as exist:
        ts[":A"].map({ts[":A"]: ts[":B"]})
    with ts.scope(":Insert"):
        ts[":B"].map({ts[":B"]: exist[":A"]})
    RegisterRule(ts)
```
But now it will create an entirely new node, effectively `B2`, and fact
`(B2, B, A)`! To resolve this, we need to explicitly tell the system that
`:BadRule:Insert:B` refers to the same node as `:BadRule:MustMap:B`. This can
be done using the `AssertNodesEqual` macro in `ts_utils.py`:
```python
with ts.scope(":BadRule"):
    with ts.scope(":MustMap") as exist:
        ts[":A"].map({ts[":A"]: ts[":B"]})
    with ts.scope(":Insert") as insert:
        ts[":B"].map({ts[":B"]: exist[":A"]})
    RegisterRule(ts)
    AssertNodesEqual(ts, [exist[":B"], insert[":B"]], "/:BadRule")
```
In fact, if the nodes in question end in the same name (after ignoring the
`:MustMap` or `:Insert` scopes), as `:MustMap:B` and `:Insert:B` do, then
`RegisterRule` can automatically assert their equality using the
`auto_assert_equal` option:
```python
with ts.scope(":BadRule"):
    with ts.scope(":MustMap") as exist:
        ts[":A"].map({ts[":A"]: ts[":B"]})
    with ts.scope(":Insert"):
        ts[":B"].map({ts[":B"]: exist[":A"]})
    RegisterRule(ts, auto_assert_equal=True)
```

### (4) Applying Update Rules
After declaring the `TripletStructure` with some initial facts and update rules, we
will initialize a new `TSRuntime` instance. The runtime will extract the rules
we declared earlier and provide an interface to actually modify the structure
using those rules. We initialize a `TSRuntime` like so:
```python
from runtime.runtime import TSRuntime

# ... initializing tc ...
rt = TSRuntime(ts)
```

`rt` exposes a somewhat lower-level API for applying rules:
`rt.get_rule(rule_name)` returns a representation of the desired update rule.
Given an update rule, `rt.propose(rule)` will yield possible assignments to the
rule of the form `(assignment, delta)`. `assignment` describes the nodes
satisfying the rule's pattern while `delta` describes the modification to the
structure which should occur according to the rule.

### (5) Tactics for Applying Update Rules
`tactic_utils.py` defines a number of helpful functions, particularly
`Fixedpoint`, for repeatedly applying a rule to the structure.

### (6) Recording Changes to the Structure
To assist with backtracking search, there are a number of tools available to
checkpoint and rollback the state of a structure, as well as record changes.
These are described in docstrings in `ts_lib.py`.
