#pragma once
#include <cstdint>

class Board
{
public:
    using Bitboard = uint64_t;

    enum Square {
        a1, b1, c1, d1, e1, f1, g1, h1,
        a2, b2, c2, d2, e2, f2, g2, h2,
        a3, b3, c3, d3, e3, f3, g3, h3,
        a4, b4, c4, d4, e4, f4, g4, h4,
        a5, b5, c5, d5, e5, f5, g5, h5,
        a6, b6, c6, d6, e6, f6, g6, h6,
        a7, b7, c7, d7, e7, f7, g7, h7,
        a8, b8, c8, d8, e8, f8, g8, h8
    };

    enum Color {
        White, Black, All
    };

    enum PieceType {
        Pawn, Knight, Bishop, Rook, Queen, King
    };

    static const char* ColorNames[3];
    static const char* PieceTypeNames[6];
    static const char* SquareNames[64];


    Board();
    virtual ~Board() = default;

    // bit operations
    static void setBit(Bitboard& bitboard, Square square);
    static void clearBit(Bitboard& bitboard, Square square);
    static bool getBit(Bitboard bitboard, Square square);
    static int countBits(Bitboard bitboard);
    static int getLSBIndex(Bitboard bitboard);


    // bitmask methods
    Bitboard maskPawnAttacks(Color color, Square square) const;
    Bitboard maskKnightAttacks(Square square) const;
    Bitboard maskBishopAttacks(Square square) const;
    Bitboard maskRookAttacks(Square square) const;
    Bitboard maskKingAttacks(Square square) const;


    // on the fly attack creation for sliding pieces
    Bitboard dynamicBishopAttacks(Square square, Bitboard blocker) const;
    Bitboard dynamicRookAttacks(Square square, Bitboard blocker) const;

    Bitboard setOccupancy(int index, int numMaskBits, Bitboard attackMask) const;

    // board position functions
    void startingPosition();

    // debug helper functions
    void printPieceboards();
    void printOccupancyboards();
    void printBitboard(Bitboard bb);

private:
    // pieceBitboards[color (white, black)][pieceType]
    Bitboard pieceBitboards[2][6];

    // occupancyBitboards[color (white, black, all)]
    Bitboard occupancyBitboards[3];

    // helper masks
    const Bitboard notFile_A = 18374403900871474942ULL;
    const Bitboard notFile_AB = 18229723555195321596ULL;
    const Bitboard notFile_H = 9187201950435737471ULL;
    const Bitboard notFile_HG = 4557430888798830399ULL;

    // pawnAttacks[color (white, black)][square]
    Bitboard pawnAttacks[2][64];

    // knightAttacks[square]
    Bitboard knightAttacks[64];

    // kingAttacks[square]
    Bitboard kingAttacks[64];

};
