#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "train.hpp"

namespace py = pybind11;

// Define the functions usable for the python module
PYBIND11_MODULE(GeneticShogi, m) {
    m.doc() = "Python3 package for Shogi agent based on genetic algorithms";

    /* m.def("evaluate", &evaluate_organism, "A function which evaluates an organism"); */

    py::class_<OrganismEvaluator>(m, "OrganismEvaluator")
        .def(py::init<>())
        .def("evaluate_organism", &OrganismEvaluator::evaluate_organism);
}
