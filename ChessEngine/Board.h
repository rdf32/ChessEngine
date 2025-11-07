#pragma once
#include <cstdint>
using Bitboard = uint64_t;

enum Square {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8, no_sq
};

enum Color {
    White, Black, All
};

enum PieceType {
    Pawn, Knight, Bishop, Rook, Queen, King
};

struct Piece {
    PieceType type;
    Color color;
};

enum CastlingType {
    wk = 1, wq = 2, bk = 4, bq = 8
};

// bit operations
bool getBit(Bitboard bitboard, Square square);
void setBit(Bitboard& bitboard, Square square);
void clearBit(Bitboard& bitboard, Square square);
int countBits(Bitboard bitboard);
int getLSBIndex(Bitboard bitboard);

// magic bitboard methods
Bitboard setOccupancy(int index, int numMaskBits, Bitboard attackMask);

class Board
{
public:

    Board();
    virtual ~Board() = default;

    // bitmask methods
    Bitboard maskPawnAttacks(Color color, Square square) const;
    Bitboard maskKnightAttacks(Square square) const;
    Bitboard maskBishopAttacks(Square square) const;
    Bitboard maskRookAttacks(Square square) const;
    Bitboard maskKingAttacks(Square square) const;


    // on the fly attack creation for sliding pieces
    Bitboard dynamicBishopAttacks(Square square, Bitboard blocker) const;
    Bitboard dynamicRookAttacks(Square square, Bitboard blocker) const;

    //Bitboard findMagicNumber(int square, int relevant_bits, int bishop);
    inline Bitboard getBishopAttacks(int square, Bitboard occupancy) const;
    inline Bitboard getRookAttacks(int square, Bitboard occupancy) const;


    // initialization methods
    void initTables();
    void setStartingPosition();
    void initLeaperPieces() const;
    //void initMagicNumbers();
    void initSliderPieces() const;

    void parseFEN(const std::string& fen);

    // debug helper methods
    void printPieceboards();
    void printOccupancyboards();
    void printBitboard(Bitboard bb);
    void printBoard() const;

private:

    // pieceBitboards[color (white, black)][pieceType]
    Bitboard pieceBitboards[2][6];

    // occupancyBitboards[color (white, black, all)]
    Bitboard occupancyBitboards[3];

    int side;
    int enpassant;
    int castle;
};
