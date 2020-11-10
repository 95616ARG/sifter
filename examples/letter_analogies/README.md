# Letter Analogy Example
This directory contains code for solving simple letter analogies with Sifter.
You can run the example from the root of the Sifter repository like so:
```bash
bazel run examples/letter_analogies:letter_analogy
```
It will print progress in solving the analogy, then print the solution `efg`.

#### Files
* `letter_analogy.py` contains code defining the letter-analogy problem as well
  as update rules which identify, e.g., nodes that are the head of a letter
  group.
* `letter_tactics.py` contains semi-general-purpose tactics for solving such
  letter analogies. It currently relies on a number of hand-picked heuristics
  to find the analogy quickly.
* `test_letter_analogy.py` is a Pytest test which ensures that
  `letter_analogy.py` returns the correct result.
