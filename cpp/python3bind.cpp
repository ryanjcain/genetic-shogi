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
        .def("set_mode", &OrganismEvaluator::set_mode)
        .def("get_mode", &OrganismEvaluator::get_mode)
        .def("set_num_eval", &OrganismEvaluator::set_num_eval)
        .def("get_num_eval", &OrganismEvaluator::get_num_eval)
        .def("evaluate_organism", &OrganismEvaluator::evaluate_organism)
        .def("get_evaluation_stats", &OrganismEvaluator::get_evaluation_stats);
}
