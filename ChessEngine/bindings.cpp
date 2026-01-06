#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include "chess.h"

namespace py = pybind11;

PYBIND11_MODULE(chess_engine, m) {
    py::class_<State>(m, "State")
        .def_readonly("side", &State::side)
        .def_readonly("castling", &State::castling)
        .def_readonly("enpassant", &State::enpassant)

        .def_property_readonly("pieces", [](const State& s) {
        return py::array_t<uint64_t>(
            { 2, 6 },                                   // shape
            { 6 * sizeof(uint64_t), sizeof(uint64_t) }, // stride
            &s.pieces[0][0]                             // base pointer
        );
            })

        .def_property_readonly("occupancy", [](const State& s) {
        return py::array_t<uint64_t>(
            { 3 },
            { sizeof(uint64_t) },
            s.occupancy
        );
            });

    py::class_<Board>(m, "Board")
        .def(py::init<>())
        .def("get_state", &Board::getState)
        .def("parse_fen", &Board::parseFEN, py::arg("fen"),
            "Parse a FEN string and set the board state accordingly");
}

