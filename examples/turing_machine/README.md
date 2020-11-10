# Turing Machine Example
This directory contains code for simulating a Turing machine with `TSLang`. Its
primary goal is to demonstrate the use and power of our rewrite rules.
You can run the example from the root of the Sifter repository like so:
```bash
bazel run examples/turing_machine:turing_machine
```
It will print the proposed delta, corresponding to one step of the machine's
execution.

#### Files
* `turing_machine.py` contains code defining the TM as a triplet structure.
* `test_turing_machine.py` is a Pytest test which ensures that
  `turing_machine.py` returns the correct result.
