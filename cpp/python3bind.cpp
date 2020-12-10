#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "train.hpp"
#include "organism-game.hpp"

namespace py = pybind11;

// Define the functions usable for the python module
PYBIND11_MODULE(GeneticShogi, m) {
    m.doc() = "Python3 package for Shogi agent based on genetic algorithms";


    // Organism evaluator class
    py::class_<OrganismEvaluator>(m, "OrganismEvaluator")
        .def(py::init<>())
        .def("set_mode", &OrganismEvaluator::set_mode)
        .def("get_mode", &OrganismEvaluator::get_mode)
        .def("set_num_eval", &OrganismEvaluator::set_num_eval)
        .def("get_num_eval", &OrganismEvaluator::get_num_eval)
        .def("get_num_features", &OrganismEvaluator::get_num_features)
        .def("get_feature_labels", &OrganismEvaluator::get_feature_labels)
        .def("get_num_major_features", &OrganismEvaluator::get_num_major_features)
        .def("evaluate_organism", &OrganismEvaluator::evaluate_organism)
        .def("get_evaluation_stats", &OrganismEvaluator::get_evaluation_stats);

    // Class to play games between two organisms
    py::class_<OrganismGame>(m, "OrganismGame")
        .def(py::init<vector<int>, vector<int>, int, int>())
        .def("simulate", &OrganismGame::simulate);
}
