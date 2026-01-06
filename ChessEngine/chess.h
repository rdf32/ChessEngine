#pragma once
#include <cstdint>
#include <vector>
#include <cstring>
#include <string>

using Bitboard = uint64_t;
using Move = uint32_t;

enum Square : uint8_t {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8, no_sq
};

enum Color : uint8_t {
    White, Black, All
};

enum PieceType : uint8_t {
    Pawn, Knight, Bishop, Rook, Queen, King
};

enum CastlingType : uint8_t {
    wk = 1, wq = 2, bk = 4, bq = 8
};

enum MoveMode : uint8_t { ALL_MOVES, CAPTURES_ONLY };

struct Piece {
    PieceType type;
    Color color;
};

struct MoveList {
    using Move = uint32_t;

    Move moves[256] = {};
    size_t count;

    MoveList();

    void add(Move move) noexcept;
    void clear() noexcept;

    size_t size() const noexcept;
    bool empty() const noexcept;

    Move operator[](size_t i) const noexcept;
    Move& operator[](size_t i) noexcept;
};

class MoveStore {
public:
    MoveStore(Move move);

    int getSource() const;
    int getTarget() const;
    int getColor() const;
    int getPiece() const;
    int getPromoted() const;
    bool isCapture() const;
    bool isDoublePush() const;
    bool isEnPassant() const;
    bool isCastling() const;


private:
    int source;
    int target;
    int color;
    int piece;
    int promoted;

    bool capture;
    bool doubleM;
    bool enpassant;
    bool castling;
};
// bit operations
bool getBit(Bitboard bitboard, Square square);
void setBit(Bitboard& bitboard, Square square);
void clearBit(Bitboard& bitboard, Square square);
int countBits(Bitboard bitboard);
int getLSBIndex(Bitboard bitboard);

// #ifdef _MSC_VER
// #define FORCEINLINE __forceinline
// #else
// #define FORCEINLINE inline __attribute__((always_inline))
// #endif

// FORCEINLINE int countBits(Bitboard b) noexcept {
// #if defined(_MSC_VER)
//     return __popcnt64(b);
// #else
//     return __builtin_popcountll(b);
// #endif
// }

// FORCEINLINE int getLSBIndex(Bitboard b) noexcept {
// #if defined(_MSC_VER)
//     unsigned long idx;
//     _BitScanForward64(&idx, b);
//     return (int)idx;
// #else
//     return __builtin_ctzll(b);
// #endif
// }

// magic bitboard methods
Bitboard setOccupancy(int index, int numMaskBits, Bitboard attackMask);
Move encodeMove(int source, int target, int color, int piece, int promoted, bool capture, bool doubleM, bool enpassant, bool castling);
void printMove(Move move);

Bitboard maskPawnAttacks(Color color, Square square);
Bitboard maskKnightAttacks(Square square);
Bitboard maskBishopAttacks(Square square);
Bitboard maskRookAttacks(Square square);
Bitboard maskKingAttacks(Square square);


// on the fly attack creation for sliding pieces
Bitboard dynamicBishopAttacks(Square square, Bitboard blocker);
Bitboard dynamicRookAttacks(Square square, Bitboard blocker);

//Bitboard findMagicNumber(int square, int relevant_bits, int bishop);
Bitboard getBishopAttacks(int square, Bitboard occupancy);
Bitboard getRookAttacks(int square, Bitboard occupancy);
Bitboard getQueenAttacks(int square, Bitboard occupancy);


struct State {
    Bitboard pieces[2][6];    // [color][piece]
    Bitboard occupancy[3];    // [white, black, both]
    int side;                 // side to move
    int castling;             // castling rights bitmask
    int enpassant;            // square or -1
};

// Board methods
class Board {
public:
    Board();
    // initialization methods
    void initTables();
    void initLeaperPieces();
    void initSliderPieces();

    // I/O methods
    void parseFEN(const std::string& fen);

    // attacking methods
    bool isSquareAttacked(Square square, Color side);
    void pawnMoves(Color side, MoveList& moveList);
    void knightMoves(Color side, MoveList& moveList);
    void bishopMoves(Color side, MoveList& moveList);
    void rookMoves(Color side, MoveList& moveList);
    void queenMoves(Color side, MoveList& moveList);
    void kingMoves(Color side, MoveList& moveList);

    // move methods
    MoveList generateMoves();
    bool makeMove(Move move, MoveMode mode);
    Move parseMove(const std::string& move_string);

    // debug helper methods
    void printPieceboards();
    void printOccupancyboards();
    void printBitboard(Bitboard bb);
    void printBoard();
    void printAttackedSquares(Color side);
    void printMoves(const MoveList& moves);

    // state methods
    State getState() const;

    // perft
    uint64_t perft_driver(int depth);
    void perft_test(int depth);

private:
    Bitboard pieceBitboards[2][6]; // [color][piece]
    Bitboard occupancyBitboards[3]; // [color]

    int side;
    int enpassant;
    int castling;
};








//binary move bits                               hexidecimal constants
//
//0000 0000 0000 0000 0011 1111    source square       0x3f
//0000 0000 0000 1111 1100 0000    target square       0xfc0
//0000 0000 0001 0000 0000 0000    color               
//0000 0000 1110 0000 0000 0000    piece               
//0000 1111 0000 0000 0000 0000    promoted piece      
//0001 0000 0000 0000 0000 0000    capture flag        0x100000
//0010 0000 0000 0000 0000 0000    double push flag    0x200000
//0100 0000 0000 0000 0000 0000    enpassant flag      0x400000
//1000 0000 0000 0000 0000 0000    castling flag       0x800000