# Program Analysis Example
This directory contains code for the Sifter demos presented in our Onward!
2020 paper.  You can run the examples from the root of the Sifter repository
like
so:
```bash
bazel run examples/program_analysis:program_understanding
bazel run examples/program_analysis:transform_learning
bazel run examples/program_analysis:api_migration
```
Each example will run, then prompt you to visit `http://localhost:8001` in a
web browser which will show the result.

#### Files
* `program_understanding.py`: Section 3.1 demonstration of comparative program
  understanding.
* `transform_learning.py`: Section 3.2 demonstration of learning to generalize
  a program optimization.
* `api_migration.py`: Section 3.3 demonstration of learning to generalize
  API migration examples.
* `lazy_structure.py`: Classes to interface between source code files and
  triplet structures.
* `analyzelib.py`: Helper methods and tactics for the demos.
* `ui/`: Interactive UI for displaying the result of the analogy-making
  demonstrations.
