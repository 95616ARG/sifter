#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "ts_lib.h"

namespace py = pybind11;

PYBIND11_MODULE(ts_cpp, m) {
  py::class_<Triplet>(m, "Triplet")
    .def(py::init<Node, Node, Node>());

  py::class_<Structure>(m, "Structure")
    .def(py::init<>())
    .def("addFact", &Structure::AddFactPy)
    .def("removeFact", &Structure::RemoveFactPy)
    .def("lookup", &Structure::Lookup);

  py::class_<Solver>(m, "Solver")
    .def(py::init<
           const Structure&,
           const size_t,
           const std::vector<Triplet>&,
           const std::vector<std::set<size_t>>
         >())
    .def("isValid", &Solver::IsValid)
    .def("nextAssignment", &Solver::NextAssignment);
}
