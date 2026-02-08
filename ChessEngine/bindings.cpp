#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include "chess.h"

namespace py = pybind11;

PYBIND11_MODULE(chess_engine, m) {
    py::class_<State>(m, "State")
        .def_readonly("side", &State::side)
        .def_readonly("castling", &State::castling)
        .def_readonly("enpassant", &State::enpassant)
        .def_readonly("in_check", &State::in_check)


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
            "Parse a FEN string and set the board state accordingly")
        .def("legal_moves", [](Board& self) {
            const MoveList moves = self.legalMoves();
            return py::array_t<uint32_t>(moves.size(), moves.moves);})
        .def("make_move",
            [](Board& self, Move move) {
                return self.makeMove(move, MoveMode::ALL_MOVES);
            },
            py::arg("move"));
}

