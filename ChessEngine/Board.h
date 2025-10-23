#pragma once
#include <cstdint>

class Board
{
public:
    using Bitboard = uint64_t;

    enum Square {
        A1, B1, C1, D1, E1, F1, G1, H1,
        A2, B2, C2, D2, E2, F2, G2, H2,
        A3, B3, C3, D3, E3, F3, G3, H3,
        A4, B4, C4, D4, E4, F4, G4, H4,
        A5, B5, C5, D5, E5, F5, G5, H5,
        A6, B6, C6, D6, E6, F6, G6, H6,
        A7, B7, C7, D7, E7, F7, G7, H7,
        A8, B8, C8, D8, E8, F8, G8, H8
    };

    enum Color {
        White, Black, All
    };

    enum PieceType {
        Pawn, Knight, Bishop, Rook, Queen, King
    };

    static const char* ColorNames[3];
    static const char* PieceTypeNames[6];

    Board();
    virtual ~Board() = default;

    // bit operations
    static void setBit(Bitboard& bb, Square sq);
    static void clearBit(Bitboard& bb, Square sq);
    static bool getBit(Bitboard bb, Square sq);

    // bitmask methods
    Bitboard pawnMask(Color color, Square square) const;
    Bitboard knightMask(Square square) const;
    Bitboard bishopMask(Square square) const;
    Bitboard rookMask(Square square) const;


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
    //Bitboard kingAttacks[64];

};
