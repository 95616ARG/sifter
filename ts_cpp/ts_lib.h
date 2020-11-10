#ifndef TS_LIB_H_
#define TS_LIB_H_

#include <array>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <stack>
#include <vector>
#include <cassert>

// Nodes are > 0. Variables are <= 0. Where Nodes are expected, an 'empty' node
// is represented by 0.

#define Node int
#define Variable int
#define NodeOrVariable int

class Triplet : public std::array<Node, 3> {
 public:
  Triplet(Node i, Node j, Node k) : std::array<Node, 3>({i, j, k}) {}
};

// https://en.cppreference.com/w/cpp/utility/hash
namespace std {
template<> struct hash<Triplet> {
  std::size_t operator()(Triplet const& triplet) const noexcept {
    std::size_t h1 = std::hash<int>{}(triplet[0]);
    std::size_t h2 = std::hash<int>{}(triplet[1]);
    std::size_t h3 = std::hash<int>{}(triplet[2]);
    // TODO(masotoud): maybe profile with other combinations.
    return h1 ^ (h2 << 1) ^ (h3 >> 1);
  }
};
}  // namespace std

class Structure {
 public:
  void AddFact(const Triplet &fact);
  void RemoveFact(const Triplet &fact);
  void AddFactPy(Node i, Node j, Node k);
  void RemoveFactPy(Node i, Node j, Node k);
  const std::vector<Triplet> &Lookup(const Triplet &fact) const;
  bool AllTrue(const std::vector<Triplet> &facts) const;
  bool IsTrue(const Triplet &fact) const;

 private:
  std::unordered_map<Triplet, std::vector<Triplet>> facts_;
  // TODO(masotoud)
  std::vector<Triplet> empty_;
};

class Solver {
 public:
  Solver(const Structure &structure,
         const size_t n_variables,
         const std::vector<Triplet> &constraints,
         const std::vector<std::set<size_t>> &maybe_equal);

  bool IsValid() { return valid_; }
  std::vector<Node> NextAssignment();
  void Assign(const Node to);
  void UnAssign();
  void GetOptions();

 private:
  int CurrentVariable() const;
  bool IsVariable(int node) const;

  struct State {
    State() : options(), options_it(options.begin()) { }
    std::set<Node> options;
    std::set<Node>::iterator options_it;
  };

  const Structure &structure_;
  const size_t n_variables_;
  bool valid_;
  std::vector<Triplet> constraints_;
  std::vector<Triplet> working_constraints_;
  // Size: n_variables
  std::vector<std::vector<size_t>> var_to_constraints_;
  // Size: n_variables
  std::vector<std::set<size_t>> may_equal_;
  // Size: n_variables
  std::vector<Node> assignment_;
  // Size: n_variables
  std::vector<State> states_;
  // Range: [0, infty)
  // NOTE: This is actually -current_variable_. Makes it more convenient for
  // indexing into assignment_, states_, etc.
  int current_index_ = 0;
};

#endif  // TS_LIB_H_
