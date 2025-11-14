#pragma once
#include <cstdint>
#include <vector>

using Bitboard = uint64_t;
using Move = uint32_t;

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

enum CastlingType {
    wk = 1, wq = 2, bk = 4, bq = 8
};

enum MoveMode { ALL_MOVES, CAPTURES_ONLY };

struct Piece {
    PieceType type;
    Color color;
};

struct MoveList {
    using Move = uint32_t; // 32-bit move encoding

    std::vector<Move> moves;

    MoveList();
    void add(Move move);
    size_t size() const noexcept;
    bool empty() const noexcept;
    void clear() noexcept;
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

// magic bitboard methods
Bitboard setOccupancy(int index, int numMaskBits, Bitboard attackMask);
Move encodeMove(int source, int target, int color, int piece, int promoted, bool capture, bool doubleM, bool enpassant, bool castling);
void printMove(Move move);

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
    Bitboard getBishopAttacks(int square, Bitboard occupancy) const;
    Bitboard getRookAttacks(int square, Bitboard occupancy) const;
    Bitboard getQueenAttacks(int square, Bitboard occupancy) const;

    // attacking methods
    bool isSquareAttacked(Square square, Color side) const;
    void pawnMoves(Color side);
    void knightMoves(Color side);
    void bishopMoves(Color side);
    void rookMoves(Color side);
    void queenMoves(Color side);
    void kingMoves(Color side);

    void generateMoves();
    void resetMoves();
    void saveState();
    void takeBack();
    bool makeMove(Move move, MoveMode mode);


    // initialization methods
    void initTables();
    //void setStartingPosition();
    void initLeaperPieces() const;
    //void initMagicNumbers();
    void initSliderPieces() const;

    // I/O methods
    void parseFEN(const std::string& fen);
    const MoveList& getMoveList() const;
    void addMove(Move move);

    // debug helper methods
    void printPieceboards();
    void printOccupancyboards();
    void printBitboard(Bitboard bb);
    void printBoard() const;
    void printAttackedSquares(Color side);
    void printMoves() const;

private:

    MoveList moves;
    Bitboard pieceBitboards[2][6];
    Bitboard occupancyBitboards[3];

    int side;
    int enpassant;
    int castle;

    Bitboard prev_pieceBitboards[2][6];
    Bitboard prev_occupancyBitboards[3];

    int prev_side;
    int prev_enpassant;
    int prev_castle;
};

//binary move bits                               hexidecimal constants
//
//0000 0000 0000 0000 0011 1111    source square       0x3f
//0000 0000 0000 1111 1100 0000    target square       0xfc0
//0000 0000 0001 0000 0000 0000    color               
//0000 0000 1110 0000 0000 0000    piece               
//0000 0001 0000 0000 0000 0000    promoted color      
//0000 1110 0000 0000 0000 0000    promoted piece      
//0001 0000 0000 0000 0000 0000    capture flag        0x100000
//0010 0000 0000 0000 0000 0000    double push flag    0x200000
//0100 0000 0000 0000 0000 0000    enpassant flag      0x400000
//1000 0000 0000 0000 0000 0000    castling flag       0x800000
