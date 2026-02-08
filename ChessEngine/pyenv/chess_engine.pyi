from __future__ import annotations
import numpy
import numpy.typing
import typing
__all__: list[str] = ['Board', 'State']
class Board:
    def __init__(self) -> None:
        ...
    def get_state(self) -> State:
        ...
    def legal_moves(self) -> numpy.typing.NDArray[numpy.uint32]:
        ...
    def make_move(self, move: typing.SupportsInt) -> bool:
        ...
    def parse_fen(self, fen: str) -> None:
        """
        Parse a FEN string and set the board state accordingly
        """
class State:
    @property
    def castling(self) -> int:
        ...
    @property
    def enpassant(self) -> int:
        ...
    @property
    def in_check(self) -> bool:
        ...
    @property
    def occupancy(self) -> numpy.typing.NDArray[numpy.uint64]:
        ...
    @property
    def pieces(self) -> numpy.typing.NDArray[numpy.uint64]:
        ...
    @property
    def side(self) -> int:
        ...
