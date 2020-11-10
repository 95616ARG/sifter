# Analogy-Utils
Python driver for interacting with the Mapper rules in [mapper.py](mapper.py).

### Running Example
We will use the following running example from
[examples/letter_analogies/letter_analogy.py](examples/letter_analogies/letter_analogy.py):
```
If abc becomes bcd and efg becomes efh, then what does ijk become?
```
For notational simplicity, we will label the nodes corresponding to letters
like so:
```
If abc becomes (b')(c')(d') and efg becomes (f')(g')(h'), then what does ijk become?
```

We will also assume facts related to the position of letters in the strings as
well as alphabetical ordering:
```
(n1, a, Left), (n1, b, Right)
(n2, b, Left), (n2, c, Right)
...
(s1, a, Pred), (s1, b, Succ)
(s2, b, Pred), (s2, c, Succ)
(s3, a, Pred), (s3, b', Succ)
(s4, b, Pred), (s4, c', Succ)
...
```
and so forth.

### Broad Idea: Joint Traversal
Our goal is to form a map identifying corresponding nodes.

If we think of the triplet structure as a graph, the basic idea behind our
approach is to do a joint, breadth-first traversal of the graph. We start by
assigning two nodes to correspond to each other, then we extend the
correspondance by following edges from each node iteratively and
simultaneously.

### Starting the Analogy
We seed the analogy by telling it that two given nodes should correspond to
each other. In fact, we tell it that two _facts_ should correspond to each
other. In this case, a pretty safe fact to start with is that `abc` and `efg`
are both letter strings that are "pre-transformation." In other words, we
want to abstract the facts:
```
(t1, abc, TransformFrom) and (t2, efg, TransformFrom)
```
to form abstract nodes `^t` and `^???` with fact:
```
(^t, ^???, TransformFrom).
```

##### In Code
In order to do this, we use the `Analogy.Begin` method, roughly like:
```
analogy = Analogy.Begin(rt,  {
    no_slip[":MA"]: t1,
    no_slip[":A"]: abc,
    no_slip[":MB"]: t2,
    no_slip[":B"]: efg,
    no_slip[":C"]: TransformFrom,
}).
```

### Extending the Start
If `extend=True` is passed to `Analogy.Begin` (the default) then it will
automatically start to build out from this fact. Essentially, it will look at
all other facts regarding `t1` and `t2` and try to lift (antiunify) them into
abstract facts. In this case, we find that there are corresponding facts:
```
(t1, b'c'd', TransformTo) and (t2, f'g'h', TransformTo)
```
Hence in the abstract we can add the node `^?'?'?'` as well as the fact:
```
(^t, ^?'?'?', TransformTo).
```

##### In Code
Again, this happens automatically in `Analogy.Begin(..., extend=True)`. At this
point, the abstraction consists of two abstract groups, `^???` and `^?'?'?'`,
where the latter is the post-transformation of the former. Hence the only
correspondance we know between the two examples so far is that they both
involve pairs of letter strings before and after the transformation. This is
all that is claimed by our initial mapping of `t1` and `t2`, hence
`Analogy.Begin` finishes.

### Pivots: Extending With New Fact Nodes
To extend the analogy further, we need to involve additional fact nodes. We do
this by pivoting off of nodes already in the analogy and identifying fact nodes
that claim similar things about nodes already mapped to each other. For
example, we might have fact nodes `h1` and `h2` expressing that `a` is the
start of string `abc` and `e` is the start of string `efg`:
```
(h1, abc, Group), (h1, a, Head)
and
(h2, efg, Group), (h2, e, Head).
```
Note that `abc` and `efg` are already mapped to each other in the analogy, and
`h1` and `h2` both claim the same thing about `abc`/`efg` (namely, that they're
groups). Hence, we can infer that `h1` and `h2` might correspond to each other,
forming a new abstract node `^h` with fact:
```
(^h, ^???, Group).
```

##### In Code
We perform this pivoting to a new fact node with the method
`Analogy.ExtendMap`:
```
analogy.ExtendMap([Group]).
```

### Building off a Fact Node
We've now recognized that both `abc` and `efg` are groups of letters, but `h1`
and `h2` also claim something else: that each group has a head letter that
starts it. Because we've mapped `h1` and `h2` to each other, we can follow this
fact as well to infer that the heads of each group should probably correspond
as well. In other words, we can lift `a` and `e` to abstract node `^1` and add
fact:
```
(^h, ^1, Head).
```

##### In Code
The call to `Analogy.ExtendMap` where we added `^h` in the first place will
automatically follow all facts of this form when possible. It does this by
calling the method `Analogy.ExtendFacts(^h)` which in turn repeatedly calls
the `NewConcrete` rule to add nodes like `^1` and `Analogy.LiftFacts(^h)` to
lift any other triplets like `(^h, ^1, Head)`.

### Summary of Analogy-Making by Traversal
In general, the operations described above are enough to create an analogy. We
pivot repeatedly between:
(i) Abstracting fact nodes that make claims about nodes already in the
analogy. E.g., `h1` and `h2` claim `abc` and `efg` (which we know correspond)
are groups, hence, `h1<->h2` is probably consistent with our analogy so we
can abstract them to `^h`.
(ii) Abstract nodes for which claims are made in those corresponding fact
nodes.  E.g., we think `h1` and `h2` correspond, and `h1` claims `a` is a head
while `h2` claims `e` is a head of corresponding groups `abc` and `efg`. Hence,
we might infer that in fact `a` and `e` correspond, forming some abstract node
`^1` which is also a head of the abstract group `^???`.

### Avoiding Bad Maps
Unfortunately, this approach can run into problems. For example, after we say
that `a` and `e` correspond, we might notice that there are fact nodes `s1` and
`m1` with facts:
```
(s1, a, Pred), (s1, b, Succ)
and
(m1, e, Pred), (m1, f, Succ).
```
We could then map `s1<->m1` and follow this to map `b<->f`, which would
work perfectly. However, there might _also_ be a fact node `m3` with facts:
```
(m3, e, Pred), (m3, f', Succ),
```
which actually maps _across groups_ `efg` and `f'g'h'`. The problem is that we
pivot to groups based only on a single triplet, and, hence, looking at only a
single triplet it's not clear if we should map
```
(s1, a, Pred)<->(m1, e, Pred)
or
(s1, a, Pred)<->(m3, e, Pred).
```
Both options look equally good when deciding whether `s1` should correspond to
`m1` or `m3`. If we pick `s1<->m1`, we saw that everything works and we map
`b<->f` as desired. However, if we map `s1<->m3` then we will infer that
actually `b<->f'`, which is probably wrong. We need some way to decide
between the two equally plausible mappings.

##### Heuristic 1: Follow Unique Claims First
The first heuristic is to avoid such scenarios when possible by following
claims which are _unique_. In this case, the problem only came about because
the first `a` was actually an alphabetical predecessor of two different nodes,
the `b` in `abc` and the `b'` in `b'c'd'`. So when we follow predecessor ->
successor, we have to make a choice of which successor we want to choose.

If we instead had followed the claim that that `a` is _to the left_ of some
other letter, there would only be one choice: the `b` in `abc`. Similarly, the
only thing to the right of the `e` is the `f` in `efg`. Hence, if we had
followed the `Left`/`Right` relation instead of `Pred`/`Succ`, we would have
arrived at the most reasonable option `b<->f`.

This generally means following _structural_ relations first, and _semantic_
relations only after that.

In code, this usually looks like calling `analogy.ExtendMap` multiple times
with different parameters, of decreasing level of uniqueness.

##### Heuristic 2: All or Nothing
Following `Left`/`Right` relations first gives us the desired correspondance of
`b<->f`. However, this doesn't immediately solve the original problem of
determining if `s1<->m1` or `s1<->m3`. Once we have decided `b<->f`, however,
we can try both `s1<->m1` and `s1<->m3` and apply our second heuristic: take
_only the fact nodes where all facts lift_. In this case, we could try to
correspond `s1<->m3` but then we would find that we could _not_ make the facts
```
(s1, b, Succ) and (m3, f', Succ)
```
correspond to each other, because we do not have `b<->f'`. Thus, `s1<->m3`
would leave facts which don't lift to the abstraction while `s1<->m1` would be
able to lift all the relavant facts. Hence, we would prefer `s1<->m1`.

In the code, this is implemented in `analogy.ExtendFacts` by calling
`analogy.LiftFacts` to try and lift all relevant facts to the abstract and
then `analogy.FactsMissing` to check if all facts were lifted. If some can't
be lifted, then `ExtendFacts` will return `False` and `ExtendMap` will give
up on that mapping.

##### Heuristic 3: Voting
In the near future we would like to take an alternate approach, which is
somewhat closer to the original Copycat: voting. Essentially, in this case we
have that following `s1<->m1` leads to a better analogy because then we can
lift all facts and it also agrees with the unique mapping when we follow
`Left`/`Right` to get `b<->f`.

### Completing an Analogy
Suppose we have already mapped `abc->bcd` and `efg->fgh` and want to start
solving `ijk->?`. We:
* First call `Analogy.Begin(..., exists=True)` to map `ijk` into the _existing_
  analogy noting correspondances between `abc->bcd` and `efg->fgh`.
* Then, we call `Analogy.ExtendMap` as before to complete the analogy between
  `ijk->?` and `abc->bcd`/`efg->fgh`.
* Then, we set `analogy.state="concretize"`.
* Then, we again call `Analogy.ExtendMap`. It will continue to traverse the
  existing analogy between `abc->bcd` and `efg->fgh`, but, because we set
  `state="concretize"`, instead of looking for nodes already in the structure
  that might correspond to abstract nodes, it just adds new nodes to the
  structure and lowers the corresponding facts from the abstract to these
  nodes.
* Finally, we run inference rules which solve those lowered facts. E.g., we
  might lower a fact that says that `_1` is the successor of `a`, then infer
  that `_1` is the letter `b`.
