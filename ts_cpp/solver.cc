#include <tuple>
#include <string>
#include "ts_lib.h"
#include <iostream>

inline int Solver::CurrentVariable() const {
  return -current_index_;
}

inline bool Solver::IsVariable(int node) const {
  return node <= 0;
}

Solver::Solver(const Structure &structure, const size_t n_variables,
               const std::vector<Triplet> &constraints,
               const std::vector<std::set<size_t>> &maybe_equal)
    : structure_(structure), n_variables_(n_variables), valid_(true),
      var_to_constraints_(n_variables, std::vector<size_t>({})),
      may_equal_(maybe_equal), assignment_(n_variables, 0),
      states_(n_variables, State()), current_index_(0) {
  assert(n_variables > 0);
  for (size_t constraint_i = 0;
       constraint_i < constraints.size();
       constraint_i++) {
    auto &constraint = constraints.at(constraint_i);
    bool any_variables = false;
    for (size_t i = 0; i < 3; i++) {
      if (IsVariable(constraint[i])) {
        var_to_constraints_.at(-constraint[i]).push_back(constraints_.size());
        any_variables = true;
      }
    }
    if (any_variables) {
      constraints_.push_back(constraint);
    } else if (!structure_.IsTrue(constraint)) {
      valid_ = false;
      break;
    }
  }
  if (valid_) {
    working_constraints_ = constraints_;
    // Initializes states_[0].
    GetOptions();
  }
}

std::vector<Node> Solver::NextAssignment() {
  if (!valid_ || n_variables_ == 0) {
    return {};
  }
  // current_index_ goes to -1 when we backtrack from the initial state.
  while (current_index_ >= 0) {
    auto &state = states_[current_index_];

    // If we have no more options for this variable, backtrack.
    if (state.options_it == state.options.end()) {
      UnAssign();
      continue;
    }

    // Otherwise, we need to pick a variable assignment and go down.
    Assign(*state.options_it);
    // Increment the pointer for the current state so the next time we get back
    // here we go on to the next one.
    // TODO(masotoud): we can roll all of this up into a do-it-all Assign()
    // method.
    state.options_it++;

    // If this is a valid assignment, return it and backtrack.
    if (current_index_ == n_variables_) {
      // TODO(masotoud): we can reorganize this so a copy isn't necessary.
      std::vector<Node> copy = assignment_;
      UnAssign();
      return copy;
    }

    // Otherwise, initialize the next state.
    GetOptions();
  }
  valid_ = false;
  return {};
}

void Solver::Assign(const Node to) {
  assignment_[current_index_] = to;
  int var = CurrentVariable();
  for (auto &i : var_to_constraints_[current_index_]) {
    for (size_t j = 0; j < 3; j++) {
      if (working_constraints_[i][j] == var) {
        working_constraints_[i][j] = to;
      }
    }
  }
  current_index_++;
}

void Solver::UnAssign() {
  // This is usually called when current_index_ in [1, n_variables_], if it's 0
  // then we're backtracking from the root node (i.e., we're done).
  current_index_--;
  if (current_index_ < 0) {
    return;
  }
  int var = CurrentVariable();
  for (auto &i : var_to_constraints_.at(current_index_)) {
    for (size_t j = 0; j < 3; j++) {
      if (constraints_[i][j] == var) {
        working_constraints_[i][j] = constraints_[i][j];
      }
    }
  }
}

void Solver::GetOptions() {
  int var = CurrentVariable();
  if (current_index_ >= n_variables_ || current_index_ < 0) {
    return;
  }
  // Set to 'true' after the first iteration. We want options to be an
  // intersection of all the local_options, so we use this to initialize it to
  // the first local_option. We could also just check options.empty(), as we
  // break once options goes empty otherwise, but I think this is a bit more
  // explicit and allows the loop to work even without the break.
  bool initialized_options = false;
  std::set<Node> &options = states_.at(current_index_).options;
  // For each constraint triplet...
  for (auto &i : var_to_constraints_.at(current_index_)) {
    // (1) Replace the variable in question with 0. E.g. if we're solving for
    // -1 and we have constraint (-1, 2, -2), we get (0, 2, 0) as emptied and
    // hole_is_var = (1, 0, 0).
    // NOTE: 0 is a variable *AS WELL AS* the indicator for an empty node. This
    // is actually not ambiguous --- empty nodes are only valid in
    // Structure::Lookup, within which variables are *in*valid.
    Triplet emptied(working_constraints_[i]);
    bool hole_is_var[3];
    for (size_t j = 0; j < 3; j++) {
      hole_is_var[j] = (emptied[j] == var);
      if (IsVariable(emptied[j])) {
        emptied[j] = 0;
      }
    }
    // (2) Look at all the matching facts and unify them to figure out what the
    // valid assignments to @var are. Note that we want a running intersection
    // with @options.
    std::set<Node> local_options;
    for (auto &triplet : structure_.Lookup(emptied)) {
      Node choice = 0;
      // In theory we can avoid this loop (and hole_is_var)
      for (size_t j = 0; j < 3; j++) {
        if (!hole_is_var[j]) {
          // This hole is not relevant to the assignment of @var. E.g. var =
          // -1, constraints = (-1, 2, -2), and the fact is (5, 4, 6) --- 6 is
          // not relevant.
          continue;
        } else if (choice == 0) {
          // This is only the case when @choice is unset.
          choice = triplet[j];
        } else if (choice != triplet[j]) {
          // There's some inconsistency.  E.g. if the constraint is (-1, 2, -1)
          // which gets mapped to emptied (0, 2, 0) which also maps against (5,
          // 6, 7). In that case, choice == 0 because 5 != 7. We can probably
          // avoid dealing with this (and hole_is_var) by expanding the memory
          // usage of Structure, but I don't think it will be worth it.
          choice = 0;
          break;
        }
      }
      // If we actually found a consistent assignment...
      if (choice > 0) {
        // We eventually want options &= local_options, so we just make
        // local_options the intersection immediately.
        if (!initialized_options || options.count(choice) > 0) {
          local_options.insert(choice);
        }
      }
    }
    options = std::move(local_options);
    initialized_options = true;
    if (options.empty()) {
      break;
    }
  }
  // (3) Check that we're not (incorrectly) re-assigning the same node to
  // different variables.
  std::set<size_t> &may_equal = may_equal_[current_index_];
  for (size_t i = 0; i < current_index_; i++) {
    if (options.count(assignment_[i]) > 0 && may_equal.count(i) == 0) {
      // We're saying it's OK to assign it to V, but already i->V and we may
      // not equal i.
      options.erase(assignment_[i]);
    }
  }
  states_[current_index_].options_it = options.begin();
}
