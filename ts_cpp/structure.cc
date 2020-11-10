#include <tuple>
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include "ts_lib.h"

void Structure::AddFact(const Triplet &fact) {
  assert(!IsTrue(fact));
  Triplet key(fact);
  for (uint8_t i = 0; i < 8; i++) {
    for (uint8_t j = 0; j < 3; j++) {
      if ((i >> j) & 0b1) {
        key[j] = fact[j];
      } else {
        key[j] = Node(0);
      }
    }
    facts_[key].push_back(fact);
  }
}

void Structure::RemoveFact(const Triplet &fact) {
  assert(IsTrue(fact));
  Triplet key(fact);
  for (uint8_t i = 0; i < 8; i++) {
    for (uint8_t j = 0; j < 3; j++) {
      if ((i >> j) & 0b1) {
        key[j] = fact[j];
      } else {
        key[j] = Node(0);
      }
    }
    auto it = std::find(facts_[key].begin(), facts_[key].end(), fact);
    assert(it != facts_[key].end());
    facts_[key].erase(it);
  }
}

void Structure::AddFactPy(Node i, Node j, Node k) {
  AddFact(Triplet(i, j, k));
}

void Structure::RemoveFactPy(Node i, Node j, Node k) {
  RemoveFact(Triplet(i, j, k));
}

const std::vector<Triplet> &Structure::Lookup(const Triplet &fact) const {
  try {
    return facts_.at(fact);
  } catch (std::out_of_range &) {
    // TODO(masotoud): Maybe not?
    return empty_;
  }
}

bool Structure::AllTrue(const std::vector<Triplet> &facts) const {
  for (auto &fact : facts) {
    if (!IsTrue(fact)) {
      return false;
    }
  }
  return true;
}

bool Structure::IsTrue(const Triplet &fact) const {
  return facts_.count(fact) > 0 && !facts_.at(fact).empty();
}
