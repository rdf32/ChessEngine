# ♟️ C++ Bitboard Chess Engine

A work-in-progress chess engine written in C++ using bitboards for fast and efficient board representation and move generation.

This project focuses on performance, correctness, and clean low-level engine design.

---

## 🚀 Features

- Bitboard-based board representation (`uint64_t`)
- Compact move encoding in a single integer
- Piece-specific bitboards (color × piece)
- Occupancy bitboards (White, Black, All)
- Legal move generation
  - Pawns, knights, bishops, rooks, queens, kings
  - Castling, en passant, promotions
- FEN parsing
- Long algebraic move parsing (`e2e4`, `e7e8q`)
- Make / TakeBack system for reversible move execution
- Perft testing for correctness and performance
- Debug utilities for printing boards and bitboards

---

## 🧠 Board Representation

The engine uses bitboards, where each bit represents a square on the chessboard:

bit 0 = a1
bit 7 = h1
bit 56 = a8
bit 63 = h8

css
Copy code

Each piece type and color has its own bitboard:

```cpp
Bitboard pieceBitboards[2][6];   // color × piece
Bitboard occupancyBitboards[3]; // White, Black, All
🛠 Debug Helpers
Utility functions are provided to visualize internal board state:

printBitboard(Bitboard bb);
printPieceboards();
printOccupancyboards();
```
🧩 Move Encoding
Moves are encoded into a single integer (Move) using bit fields for speed and compactness.

Bit Layout
Bits	Description
0–5	Source square
6–11	Target square
12	Side to move
13–15	Moving piece
16–18	Promoted piece
Flags	Capture, double push, en passant, castling

Example
```cpp
Move encodeMove(
    int source,
    int target,
    int color,
    int piece,
    int promoted,
    bool capture   = false,
    bool doubleM   = false,
    bool enpassant = false,
    bool castling  = false
);
```
This compact representation allows fast move generation, cheap copying, and efficient undo operations.

📦 MoveList
Moves are stored in a fixed-size array for performance and cache efficiency.

```cpp
class MoveList {
public:
    MoveList();
    void add(Move move) noexcept;
    void clear() noexcept;
    size_t size() const noexcept;
    bool empty() const noexcept;

    Move operator[](size_t i) const noexcept;
    Move& operator[](size_t i) noexcept;

private:
    Move moves[256];
    size_t count;
};
```
🔍 Perft Testing
The engine includes a recursive perft driver to validate move generation correctness.

```cpp
perft_test(depth);
```
Perft results can be compared against known reference values to detect bugs in move generation.

♜ FEN Support
Arbitrary positions can be loaded using Forsyth–Edwards Notation:

```cpp
parseFEN("r3k2r/p11pqpb1/bn2pnp1/2pPN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq c6 0 1");
```

⌨️ Move Input
Moves are parsed using long algebraic notation.

Examples:

e2e4
e7e8q
```cpp
Copy code
Move move = parse_move("d5c6");
Illegal or invalid moves return 0.
```
🛠️ Building
Example build command using g++:

```bash
g++ -O3 -std=c++20 *.cpp -o chessEngine
Compiler flags can be adjusted depending on platform and optimization needs.
```
🧪 Example Usage
```cpp
parseFEN(start_position);
printBoard();

Move move = parse_move("e2e4");
if (move) {
    makeMove(move, ALL_MOVES);
    printBoard();
}
```

📌 Project Status
✅ Implemented
Bitboard board representation

Compact move encoding

Legal move generation

FEN parsing

Perft testing

⏳ Planned
Search (Minimax / Alpha-Beta)

Evaluation function

UCI protocol support

📄 License
This project is open-source and intended for learning, experimentation, and engine development.
