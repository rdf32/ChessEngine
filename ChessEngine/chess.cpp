#include <cstdint>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <array>
#include <windows.h>

#include "chess.h"
#include "logger.h"

// FEN dedug positions
constexpr auto empty_board = "8/8/8/8/8/8/8/8 w - - ";
constexpr auto start_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ";
constexpr auto tricky_position = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ";
constexpr auto killer_position = "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1";
constexpr auto cmk_position = "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 ";

//Bit index : Square: bigger number left << smaller >> right
//56 57 58 59 60 61 62 63 ->a8 to h8
//48 49 50 51 52 53 54 55 ->a7 to h7
//40 41 42 43 44 45 46 47 ->a6 to h6
//32 33 34 35 36 37 38 39 ->a5 to h5
//24 25 26 27 28 29 30 31 ->a4 to h4
//16 17 18 19 20 21 22 23 ->a3 to h3
//8  9  10 11 12 13 14 15 ->a2 to h2
//0  1  2  3  4  5  6  7  ->a1 to h1

Logger logger(Logger::Level::INFO);

const char* ColorNames[3] = { "White", "Black", "All" };
const char* PieceTypeNames[6] = { "Pawn", "Knight", "Bishop", "Rook", "Queen", "King" };
const char* PromotedPieces[6] = { " ", "k", "b", "r", "q", " "};
const char* SquareNames[64] = {
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
    };

const char* PieceSymbols[2][6] = { 
    { "P", "N", "B", "R", "Q", "K" },
    { "p", "n", "b", "r", "q", "k" } 
};

const std::array<Piece, 256> symbolToPiece = [] {
    std::array<Piece, 256> arr{};

    arr['P'] = { Pawn,   White };
    arr['N'] = { Knight, White };
    arr['B'] = { Bishop, White };
    arr['R'] = { Rook,   White };
    arr['Q'] = { Queen,  White };
    arr['K'] = { King,   White };

    arr['p'] = { Pawn,   Black };
    arr['n'] = { Knight, Black };
    arr['b'] = { Bishop, Black };
    arr['r'] = { Rook,   Black };
    arr['q'] = { Queen,  Black };
    arr['k'] = { King,   Black };

    return arr;
    }();

// Bit masks
constexpr uint32_t FROM_SQ_MASK     = 0x00003F;      // bits 0–5
constexpr uint32_t TO_SQ_MASK       = 0x000FC0;      // bits 6–11
constexpr uint32_t COLOR_MASK       = 0x001000;      // bit 16
constexpr uint32_t PIECE_MASK       = 0x00E000;      // bits 12–15
constexpr uint32_t PROMO_MASK       = 0x0F0000;      // bits 16–19
constexpr uint32_t CAPTURE_FLAG     = 0x100000;      // bit 20
constexpr uint32_t DOUBLE_FLAG      = 0x200000;      // bit 21
constexpr uint32_t ENPASSANT_FLAG   = 0x400000;      // bit 22
constexpr uint32_t CASTLE_FLAG      = 0x800000;

// castling rights update constants
const int castling_rights[64] = {
    13, 15, 15, 15, 12, 15, 15, 14,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    7,  15, 15, 15,  3, 15, 15, 11
};

Bitboard pieceBitboards[2][6];
Bitboard occupancyBitboards[3];

int side;
int enpassant;
int castling;

// preserve board state
#define saveState()                                                        \
    Bitboard prev_pieceBitboards[2][6], prev_occupancyBitboards[3];         \
    int prev_side, prev_enpassant, prev_castling;                           \
    memcpy(prev_pieceBitboards, pieceBitboards, 96);                        \
    memcpy(prev_occupancyBitboards, occupancyBitboards, 24);                \
    prev_side = side, prev_enpassant = enpassant, prev_castling = castling; \

// restore board state
#define takeBack()                                                         \
    memcpy(pieceBitboards, prev_pieceBitboards, 96);                        \
    memcpy(occupancyBitboards, prev_occupancyBitboards, 24);                \
    side = prev_side, enpassant = prev_enpassant, castling = prev_castling; \

// pseudo random number state
unsigned int random_state = 1804289383;

// generate 32-bit pseudo legal numbers
unsigned int get_random_U32_number() {
    // get current state
    unsigned int number = random_state;

    // XOR shift algorithm
    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;

    // update random number state
    random_state = number;

    // return random number
    return number;
}

// generate 64-bit pseudo legal numbers
Bitboard get_random_U64_number() {
    // define 4 random numbers
    Bitboard n1, n2, n3, n4;

    // init random numbers slicing 16 bits from MS1B side
    n1 = (Bitboard)(get_random_U32_number()) & 0xFFFF;
    n2 = (Bitboard)(get_random_U32_number()) & 0xFFFF;
    n3 = (Bitboard)(get_random_U32_number()) & 0xFFFF;
    n4 = (Bitboard)(get_random_U32_number()) & 0xFFFF;

    // return random number
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

// generate magic number candidate
Bitboard generate_magic_number() {
    return get_random_U64_number() & get_random_U64_number() & get_random_U64_number();
}

void setBit(Bitboard& bitboard, Square square) {
    bitboard |= (1ULL << square);
}

void clearBit(Bitboard& bitboard, Square square) {
    bitboard &= ~(1ULL << square);
}

bool getBit(Bitboard bitboard, Square square) {
    return bitboard & (1ULL << square);
}

int countBits(Bitboard bitboard) {

    int count = 0;

    while (bitboard) {
        count++;
        bitboard &= bitboard - 1;
    }
    return count;
}

int getLSBIndex(Bitboard bitboard) {

    if (bitboard) {
        return countBits((bitboard & (~bitboard + 1)) - 1);
    }
    else {
        return -1;
    }
}

Bitboard setOccupancy(int index, int numMaskBits, Bitboard attackMask) {

    Bitboard occupancy = 0ULL;

    for (int count = 0; count < numMaskBits; count++) {

        int square = getLSBIndex(attackMask);
        clearBit(attackMask, static_cast<Square>(square));

        if (index & (1 << count)) {
            setBit(occupancy, static_cast<Square>(square));
        }
    }
    return occupancy;
}

static const int relevantBitcountBishop[64] = {
        6, 5, 5, 5, 5, 5, 5, 6,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        6, 5, 5, 5, 5, 5, 5, 6
};

static const int relevantBitcountRook[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};

// helper masks
static const Bitboard notFile_A = 18374403900871474942ULL;
static const Bitboard notFile_AB = 18229723555195321596ULL;
static const Bitboard notFile_H = 9187201950435737471ULL;
static const Bitboard notFile_HG = 4557430888798830399ULL;

// pawnAttacks[color (white, black)][square]
static Bitboard pawnAttacks[2][64];

// knightAttacks[square]
static Bitboard knightAttacks[64];

// kingAttacks[square]
static Bitboard kingAttacks[64];

//// bishop attack masks
static Bitboard bishop_masks[64];

//// rook attack masks
static Bitboard rook_masks[64];

//// bishop attacks table [square][occupancies]
static Bitboard bishop_attacks[64][512];

//// rook attacks rable [square][occupancies]
static Bitboard rook_attacks[64][4096];

static const Bitboard rook_magic_numbers[64] = {
    0x8a80104000800020ULL,
    0x140002000100040ULL,
    0x2801880a0017001ULL,
    0x100081001000420ULL,
    0x200020010080420ULL,
    0x3001c0002010008ULL,
    0x8480008002000100ULL,
    0x2080088004402900ULL,
    0x800098204000ULL,
    0x2024401000200040ULL,
    0x100802000801000ULL,
    0x120800800801000ULL,
    0x208808088000400ULL,
    0x2802200800400ULL,
    0x2200800100020080ULL,
    0x801000060821100ULL,
    0x80044006422000ULL,
    0x100808020004000ULL,
    0x12108a0010204200ULL,
    0x140848010000802ULL,
    0x481828014002800ULL,
    0x8094004002004100ULL,
    0x4010040010010802ULL,
    0x20008806104ULL,
    0x100400080208000ULL,
    0x2040002120081000ULL,
    0x21200680100081ULL,
    0x20100080080080ULL,
    0x2000a00200410ULL,
    0x20080800400ULL,
    0x80088400100102ULL,
    0x80004600042881ULL,
    0x4040008040800020ULL,
    0x440003000200801ULL,
    0x4200011004500ULL,
    0x188020010100100ULL,
    0x14800401802800ULL,
    0x2080040080800200ULL,
    0x124080204001001ULL,
    0x200046502000484ULL,
    0x480400080088020ULL,
    0x1000422010034000ULL,
    0x30200100110040ULL,
    0x100021010009ULL,
    0x2002080100110004ULL,
    0x202008004008002ULL,
    0x20020004010100ULL,
    0x2048440040820001ULL,
    0x101002200408200ULL,
    0x40802000401080ULL,
    0x4008142004410100ULL,
    0x2060820c0120200ULL,
    0x1001004080100ULL,
    0x20c020080040080ULL,
    0x2935610830022400ULL,
    0x44440041009200ULL,
    0x280001040802101ULL,
    0x2100190040002085ULL,
    0x80c0084100102001ULL,
    0x4024081001000421ULL,
    0x20030a0244872ULL,
    0x12001008414402ULL,
    0x2006104900a0804ULL,
    0x1004081002402ULL
};
static const Bitboard bishop_magic_numbers[64] = {
    0x40040844404084ULL,
    0x2004208a004208ULL,
    0x10190041080202ULL,
    0x108060845042010ULL,
    0x581104180800210ULL,
    0x2112080446200010ULL,
    0x1080820820060210ULL,
    0x3c0808410220200ULL,
    0x4050404440404ULL,
    0x21001420088ULL,
    0x24d0080801082102ULL,
    0x1020a0a020400ULL,
    0x40308200402ULL,
    0x4011002100800ULL,
    0x401484104104005ULL,
    0x801010402020200ULL,
    0x400210c3880100ULL,
    0x404022024108200ULL,
    0x810018200204102ULL,
    0x4002801a02003ULL,
    0x85040820080400ULL,
    0x810102c808880400ULL,
    0xe900410884800ULL,
    0x8002020480840102ULL,
    0x220200865090201ULL,
    0x2010100a02021202ULL,
    0x152048408022401ULL,
    0x20080002081110ULL,
    0x4001001021004000ULL,
    0x800040400a011002ULL,
    0xe4004081011002ULL,
    0x1c004001012080ULL,
    0x8004200962a00220ULL,
    0x8422100208500202ULL,
    0x2000402200300c08ULL,
    0x8646020080080080ULL,
    0x80020a0200100808ULL,
    0x2010004880111000ULL,
    0x623000a080011400ULL,
    0x42008c0340209202ULL,
    0x209188240001000ULL,
    0x400408a884001800ULL,
    0x110400a6080400ULL,
    0x1840060a44020800ULL,
    0x90080104000041ULL,
    0x201011000808101ULL,
    0x1a2208080504f080ULL,
    0x8012020600211212ULL,
    0x500861011240000ULL,
    0x180806108200800ULL,
    0x4000020e01040044ULL,
    0x300000261044000aULL,
    0x802241102020002ULL,
    0x20906061210001ULL,
    0x5a84841004010310ULL,
    0x4010801011c04ULL,
    0xa010109502200ULL,
    0x4a02012000ULL,
    0x500201010098b028ULL,
    0x8040002811040900ULL,
    0x28000010020204ULL,
    0x6000020202d0240ULL,
    0x8918844842082200ULL,
    0x4010011029020020ULL

};

// Board() {   
//     // initialize piece bitboards to 0  
//     initTables();
//     // initialize attack tables for leaper pieces (Pawn, Knight, King)
//     initLeaperPieces();
//     // initialize attack tables for sliding pieces (Bishop, Rook, Queen)
//     initSliderPieces();
// }

Bitboard maskPawnAttacks(Color color, Square square) {

    Bitboard bitboard = 0ULL;
    Bitboard attacks = 0ULL;

    setBit(bitboard, square);
    if (color == White) {

        if ((bitboard << 7) & notFile_H) { attacks |= (bitboard << 7); }
        if ((bitboard << 9) & notFile_A) { attacks |= (bitboard << 9); }

    }
    else {
        if ((bitboard >> 7) & notFile_A) { attacks |= (bitboard >> 7); }
        if ((bitboard >> 9) & notFile_H) { attacks |= (bitboard >> 9); }
    }

    return attacks;
}

Bitboard maskKnightAttacks(Square square) {

    Bitboard bitboard = 0ULL;
    Bitboard attacks = 0ULL;

    setBit(bitboard, square);
    if ((bitboard << 6) & notFile_HG) { attacks |= (bitboard << 6); }   // 2 files left (spatially)
    if ((bitboard << 15) & notFile_H) { attacks |= (bitboard << 15); }  // 1 file left (spatially)
    if ((bitboard << 17) & notFile_A) { attacks |= (bitboard << 17); }  // 1 file right (spatially)
    if ((bitboard << 10) & notFile_AB) { attacks |= (bitboard << 10); } // 2 files right (spatially)

    if ((bitboard >> 6) & notFile_AB) { attacks |= (bitboard >> 6); }   // 2 files right (spatially)
    if ((bitboard >> 15) & notFile_A) { attacks |= (bitboard >> 15); }  // 1 file right (spatially)
    if ((bitboard >> 17) & notFile_H) { attacks |= (bitboard >> 17); }  // 1 file left (spatially)
    if ((bitboard >> 10) & notFile_HG) { attacks |= (bitboard >> 10); } // 2 files left (spatially)

    return attacks;
}

Bitboard maskKingAttacks(Square square) {

    Bitboard bitboard = 0ULL;
    Bitboard attacks = 0ULL;

    setBit(bitboard, square);
    if ((bitboard << 7) & notFile_H) { attacks |= (bitboard << 7); }
    if ((bitboard << 9) & notFile_A) { attacks |= (bitboard << 9); }
    if ((bitboard >> 7) & notFile_A) { attacks |= (bitboard >> 7); }
    if ((bitboard >> 9) & notFile_H) { attacks |= (bitboard >> 9); }

    if (bitboard << 8) { attacks |= (bitboard << 8); }
    if (bitboard >> 8) { attacks |= (bitboard >> 8); }
    if ((bitboard << 1) & notFile_A) { attacks |= (bitboard << 1); }
    if ((bitboard >> 1) & notFile_H) { attacks |= (bitboard >> 1); }

    return attacks;
}

Bitboard maskBishopAttacks(Square square) {

    Bitboard attacks = 0ULL;

    int rank, file;
    int target_rank = square / 8;
    int target_file = square % 8;

    for (rank = target_rank + 1, file = target_file + 1; rank <= 6 && file <= 6; rank++, file++) { setBit(attacks, static_cast<Square>(rank * 8 + file)); }
    for (rank = target_rank + 1, file = target_file - 1; rank <= 6 && file >= 1; rank++, file--) { setBit(attacks, static_cast<Square>(rank * 8 + file)); }
    for (rank = target_rank - 1, file = target_file + 1; rank >= 1 && file <= 6; rank--, file++) { setBit(attacks, static_cast<Square>(rank * 8 + file)); }
    for (rank = target_rank - 1, file = target_file - 1; rank >= 1 && file >= 1; rank--, file--) { setBit(attacks, static_cast<Square>(rank * 8 + file)); }

    return attacks;
}

Bitboard maskRookAttacks(Square square) {

    Bitboard attacks = 0ULL;

    int rank, file;
    int target_rank = square / 8;
    int target_file = square % 8;

    for (rank = target_rank + 1; rank <= 6; rank++) { setBit(attacks, static_cast<Square>(rank * 8 + target_file)); }
    for (rank = target_rank - 1; rank >= 1; rank--) { setBit(attacks, static_cast<Square>(rank * 8 + target_file)); }
    for (file = target_file + 1; file <= 6; file++) { setBit(attacks, static_cast<Square>(target_rank * 8 + file)); }
    for (file = target_file - 1; file >= 1; file--) { setBit(attacks, static_cast<Square>(target_rank * 8 + file)); }

    return attacks;
}
Bitboard dynamicBishopAttacks(Square square, Bitboard blocker) {

    Bitboard attacks = 0ULL;

    int rank, file;
    int target_rank = square / 8;
    int target_file = square % 8;

    for (rank = target_rank + 1, file = target_file + 1; rank <= 7 && file <= 7; rank++, file++) { 
        setBit(attacks, static_cast<Square>(rank * 8 + file));
        if ((1ULL << (rank * 8 + file)) & blocker) { break; }
    }
    for (rank = target_rank + 1, file = target_file - 1; rank <= 7 && file >= 0; rank++, file--) { 
        setBit(attacks, static_cast<Square>(rank * 8 + file));
        if ((1ULL << (rank * 8 + file)) & blocker) { break; }
    }
    for (rank = target_rank - 1, file = target_file + 1; rank >= 0 && file <= 7; rank--, file++) { 
        setBit(attacks, static_cast<Square>(rank * 8 + file));
        if ((1ULL << (rank * 8 + file)) & blocker) { break; }
    }
    for (rank = target_rank - 1, file = target_file - 1; rank >= 0 && file >= 0; rank--, file--) { 
        setBit(attacks, static_cast<Square>(rank * 8 + file));
        if ((1ULL << (rank * 8 + file)) & blocker) { break; } // these could be getBits calls instead of written out
    }

    return attacks;
}

Bitboard dynamicRookAttacks(Square square, Bitboard blocker) {

    Bitboard attacks = 0ULL;

    int rank, file;
    int target_rank = square / 8;
    int target_file = square % 8;

    for (rank = target_rank + 1; rank <= 7; rank++) { 
        setBit(attacks, static_cast<Square>(rank * 8 + target_file));
        if ((1ULL << (rank * 8 + target_file)) & blocker) { break; }
    }
    for (rank = target_rank - 1; rank >= 0; rank--) { 
        setBit(attacks, static_cast<Square>(rank * 8 + target_file));
        if ((1ULL << (rank * 8 + target_file)) & blocker) { break; }
    }
    for (file = target_file + 1; file <= 7; file++) { 
        setBit(attacks, static_cast<Square>(target_rank * 8 + file));
        if ((1ULL << (target_rank * 8 + file)) & blocker) { break; }
    }
    for (file = target_file - 1; file >= 0; file--) { 
        setBit(attacks, static_cast<Square>(target_rank * 8 + file));
        if ((1ULL << (target_rank * 8 + file)) & blocker) { break; }
    }

    return attacks;
}

// initialization methods //
void initTables() {
    for (int piece = Pawn; piece <= King; piece++) {
        pieceBitboards[White][piece] = 0ULL;
        pieceBitboards[Black][piece] = 0ULL;
    }
    // initialize occupancy bitboards to 0
    for (int color = White; color <= All; color++) {
        occupancyBitboards[color] = 0ULL;
    }
}

void initLeaperPieces() {
    for (int square = a1; square <= h8; square++) {

        // initialize pawn attacks pawnAttacks[color][square]
        int rank = square / 8;
        if (!pawnAttacks[White][square] && rank != 7){
            logger.debug("init white pawn attack table for square: " + std::to_string(square) + " rank: " + std::to_string(rank));
            pawnAttacks[White][square] = maskPawnAttacks(White, static_cast<Square>(square));
        }
        
        if (!pawnAttacks[Black][square] && rank != 0) {
            logger.debug("init black pawn attack table for square: " + std::to_string(square) + " rank: " + std::to_string(rank));
            pawnAttacks[Black][square] = maskPawnAttacks(Black, static_cast<Square>(square));
        }

        if (!knightAttacks[square]) {
            // inititalize knight attacks knightAttacks[square]
            knightAttacks[square] = maskKnightAttacks(static_cast<Square>(square));
            logger.debug("init knight attack table for square: " + std::to_string(square));
        }

        if (!kingAttacks[square]) {
            // inititalize king attacks kingAttacks[square]
            kingAttacks[square] = maskKingAttacks(static_cast<Square>(square));
            logger.debug("init king attack table for square: " + std::to_string(square));
        }
    }
}

void initSliderPieces() {

    for (int square = a1; square <= h8; square++) {
        if (!bishop_masks[square]) {
            bishop_masks[square] = maskBishopAttacks(static_cast<Square>(square));

            Bitboard attack_mask = bishop_masks[square];
            int relevantBitsCount = countBits(attack_mask);
            int occupancyInds = (1 << relevantBitsCount);

            logger.debug("Occupancy Inds: " + std::to_string(occupancyInds));

            for (int index = 0; index < occupancyInds; index++) {

                Bitboard occupancy = setOccupancy(index, relevantBitsCount, attack_mask);
                int magicIndex = (occupancy * bishop_magic_numbers[square]) >> (64 - relevantBitcountBishop[square]);

                bishop_attacks[square][magicIndex] = dynamicBishopAttacks(static_cast<Square>(square), occupancy);
            }
            logger.debug("init bishop attack tables for square: " + std::to_string(square));
        }
    }

    for (int square = a1; square <= h8; square++) {
        if (!rook_masks[square]) {
            rook_masks[square] = maskRookAttacks(static_cast<Square>(square));

            Bitboard attack_mask = rook_masks[square];
            int relevantBitsCount = countBits(attack_mask);
            int occupancyInds = (1 << relevantBitsCount);

            logger.debug("Occupancy Inds: " + std::to_string(occupancyInds));

            for (int index = 0; index < occupancyInds; index++) {

                Bitboard occupancy = setOccupancy(index, relevantBitsCount, attack_mask);
                int magicIndex = (occupancy * rook_magic_numbers[square]) >> (64 - relevantBitcountRook[square]);

                rook_attacks[square][magicIndex] = dynamicRookAttacks(static_cast<Square>(square), occupancy);
            }
            logger.debug("init rook attack tables for square: " + std::to_string(square));
        }
    }
}

// get bishop attacks
Bitboard getBishopAttacks(int square, Bitboard occupancy) {
    // get bishop attacks assuming current board occupancy
    occupancy &= bishop_masks[square];
    occupancy *= bishop_magic_numbers[square];
    occupancy >>= 64 - relevantBitcountBishop[square];

    // return bishop attacks
    return bishop_attacks[square][occupancy];
}

// get rook attacks
Bitboard getRookAttacks(int square, Bitboard occupancy) {
    // get rook attacks assuming current board occupancy
    occupancy &= rook_masks[square];
    occupancy *= rook_magic_numbers[square];
    occupancy >>= 64 - relevantBitcountRook[square];

    // return rook attacks
    return rook_attacks[square][occupancy];
}

Bitboard getQueenAttacks(int square, Bitboard occupancy) {

    Bitboard diagonalAttacks = getBishopAttacks(square, occupancy);
    Bitboard straightAttacks = getRookAttacks(square, occupancy);

    return diagonalAttacks | straightAttacks;
}

void parseFEN(const std::string& fen) {

    side = White;
    enpassant = no_sq;
    castling = 0;

    memset(pieceBitboards, 0ULL, sizeof(pieceBitboards));
    memset(occupancyBitboards, 0ULL, sizeof(occupancyBitboards));

    std::string boardT, sideT, castleT, enpassantT;
    int halfmoveclock, fullmovenumber;

    std::istringstream ss(fen);
    ss >> boardT >> sideT >> castleT >> enpassantT >> halfmoveclock >> fullmovenumber;

    // Parse board
    int rank = 7;
    int file = 0;
    for (char c : boardT) {
        if (c == '/') {
            rank--;
            file = 0;
        }
        else if (isdigit(c)) {
            file += c - '0';
        }
        else {
            int square = rank * 8 + file;
            Piece piece = symbolToPiece[c];
            setBit(pieceBitboards[piece.color][piece.type], static_cast<Square>(square));
            file++;
        }
    }

    // Parse side to move
    side = (sideT == "w") ? White : Black;

    // Parse castling rights
    for (char c : castleT) {
        switch (c) {
        case 'K': castling |= wk; break;
        case 'Q': castling |= wq; break;
        case 'k': castling |= bk; break;
        case 'q': castling |= bq; break;
        case '-': break;
        default: break;
        }
    }

    // Parse en passant
    if (enpassantT != "-") {
        int ep_file = enpassantT[0] - 'a';
        int ep_rank = enpassantT[1] - '1';
        enpassant = ep_rank * 8 + ep_file;
    }
    else {
        enpassant = no_sq;
    }

    // Set occupancy boards
    for (int color = White; color <= Black; color++) {
        for (int piece = Pawn; piece <= King; piece++) {
            occupancyBitboards[color] |= pieceBitboards[color][piece];
        }
    }
    occupancyBitboards[All] |= occupancyBitboards[White];
    occupancyBitboards[All] |= occupancyBitboards[Black];

    printBoard();
}

bool isSquareAttacked(Square square, Color side) {

    // std::cout << "Checking if square " << square << " is attacked by side " << (side == White ? "White" : "Black") << std::endl;
    // check if pawn attacks - reverse thinking -- if black pawn attack hits white pawn -- then that sqaure is attacked by white pawn
    if (pawnAttacks[!side][square] & pieceBitboards[side][Pawn]) { return true; }

    // check if knight attacks
    if (knightAttacks[square] & pieceBitboards[side][Knight]) { return true; }

    // check if king attacks
    if (kingAttacks[square] & pieceBitboards[side][King]) { return true; }

    // check if bishop attacks
    if (getBishopAttacks(square, occupancyBitboards[All]) & pieceBitboards[side][Bishop]) { return true; }

    // check if rook attacks
    if (getRookAttacks(square, occupancyBitboards[All]) & pieceBitboards[side][Rook]) { return true; }

    // check if queen attacks
    if (getQueenAttacks(square, occupancyBitboards[All]) & pieceBitboards[side][Queen]) { return true; }

    return false;
}

void pawnMoves(Color side, MoveList& moveList) {
    Bitboard bitboard, attacks;
    int source_square, target_square;

    int square_offset = (side == White) ? 8 : -8;
    int start_rank_left = (side == White) ? a2 : a7;
    int start_rank_right = (side == White) ? h2 : h7;
    int promo_rank_left = (side == White) ? a7 : a2;
    int promo_rank_right = (side == White) ? h7 : h2;

    bitboard = pieceBitboards[side][Pawn];

    while (bitboard) {
        
        source_square = getLSBIndex(bitboard);
        target_square = source_square + square_offset;

        if (target_square < 0 || target_square > 63) { continue; }

        // generate quite pawn moves
        if (!getBit(occupancyBitboards[All], static_cast<Square>(target_square))) {
            // pawn promotion
            if (source_square >= promo_rank_left && source_square <= promo_rank_right) {
                moveList.add(encodeMove(source_square, target_square, side, Pawn, Queen, false, false, false, false));
                moveList.add(encodeMove(source_square, target_square, side, Pawn, Rook, false, false, false, false));
                moveList.add(encodeMove(source_square, target_square, side, Pawn, Bishop, false, false, false, false));
                moveList.add(encodeMove(source_square, target_square, side, Pawn, Knight, false, false, false, false));
            } 
            else {
                // one square ahead pawn move
                moveList.add(encodeMove(source_square, target_square, side, Pawn, 0, false, false, false, false));
                // two squares ahead pawn move
                if ((source_square >= start_rank_left && source_square <= start_rank_right) &&
                    !getBit(occupancyBitboards[All], static_cast<Square>(target_square + square_offset))) {
                    moveList.add(encodeMove(source_square, target_square + square_offset, side, Pawn, 0, false, true, false, false));
                }
            }
        }
        // get the attack moves for the selected pawn
        attacks = pawnAttacks[side][source_square] & occupancyBitboards[!side];
        while (attacks) {
            target_square = getLSBIndex(attacks);
            // pawn promotion
            if (source_square >= promo_rank_left && source_square <= promo_rank_right) {
                moveList.add(encodeMove(source_square, target_square, side, Pawn, Queen, true, false, false, false));
                moveList.add(encodeMove(source_square, target_square, side, Pawn, Rook, true, false, false, false));
                moveList.add(encodeMove(source_square, target_square, side, Pawn, Bishop, true, false, false, false));
                moveList.add(encodeMove(source_square, target_square, side, Pawn, Knight, true, false, false, false));
            }
            else {
                // one square ahead pawn move
                moveList.add(encodeMove(source_square, target_square, side, Pawn, 0, true, false, false, false));
            }
            // remove attacked piece from pawn attacks
            clearBit(attacks, static_cast<Square>(target_square));
        }
        // generate enpassant captures on a per piece basis so here for this single piece then next piece next iteration
        if (enpassant != no_sq) {
            // lookup pawn attacks and bitwise AND with enpassant square (bit)
            Bitboard enpassant_attacks = pawnAttacks[side][source_square] & (1ULL << enpassant);
            // make sure enpassant capture available
            if (enpassant_attacks) {
                // init enpassant capture target square
                int target_enpassant = getLSBIndex(enpassant_attacks);
                moveList.add(encodeMove(source_square, target_enpassant, side, Pawn, 0, true, false, true, false));
            }
        }
        // remove pawn from current pawns on board
        clearBit(bitboard, static_cast<Square>(source_square));
    }
}

void kingMoves(Color side, MoveList& moveList) {
    Bitboard bitboard, attacks;
    int source_square, target_square;

    bitboard = pieceBitboards[side][King];

    while (bitboard) {

        source_square = getLSBIndex(bitboard);
        attacks = kingAttacks[source_square] & ~occupancyBitboards[side];

        while (attacks) {

            target_square = getLSBIndex(attacks);
            // quiet move
            if (!getBit(occupancyBitboards[!side], static_cast<Square>(target_square))) {
                moveList.add(encodeMove(source_square, target_square, side, King, 0, false, false, false, false));
            }
            else {
                // captures
                moveList.add(encodeMove(source_square, target_square, side, King, 0, true, false, false, false));
            }

            clearBit(attacks, static_cast<Square>(target_square));
        }
        clearBit(bitboard, static_cast<Square>(source_square));
    }
    // castling moves
    if (side == White) {
        if (castling & wk) {
            // make sure square between king and king's rook are empty
            if (!getBit(occupancyBitboards[All], f1) && !getBit(occupancyBitboards[All], g1))
            {
                // make sure king and the f1 squares are not under attacks
                if (!isSquareAttacked(e1, Black) && !isSquareAttacked(f1, Black)) {
                    moveList.add(encodeMove(e1, g1, side, King, 0, false, false, false, true));
                    //printf("e1g1  castling move\n");
                }
            }
        }
        if (castling & wq) {
            // make sure square between king and queen's rook are empty
            if (!getBit(occupancyBitboards[All], d1) && !getBit(occupancyBitboards[All], c1) && !getBit(occupancyBitboards[All], b1)) {
                // make sure king and the d1 squares are not under attacks
                if (!isSquareAttacked(e1, Black) && !isSquareAttacked(d1, Black)) {
                    moveList.add(encodeMove(e1, c1, side, King, 0, false, false, false, true));
                }
            }
        }
    }
    else {
    
        if (castling & bk) {
            // make sure square between king and king's rook are empty
            if (!getBit(occupancyBitboards[All], f8) && !getBit(occupancyBitboards[All], g8))
            {
                // make sure king and the f8 squares are not under attacks
                if (!isSquareAttacked(e8, White) && !isSquareAttacked(f8, White)) {
                    moveList.add(encodeMove(e8, g8, side, King, 0, false, false, false, true));
                }
            }
        }
        if (castling & bq)
        {
            // make sure square between king and queen's rook are empty
            if (!getBit(occupancyBitboards[All], d8) && !getBit(occupancyBitboards[All], c8) && !getBit(occupancyBitboards[All], b8))
            {
                // make sure king and the d8 squares are not under attacks
                if (!isSquareAttacked(e8, White) && !isSquareAttacked(d8, White)) {
                    moveList.add(encodeMove(e8, c8, side, King, 0, false, false, false, true));
                }
            }
        }
    }
}

void knightMoves(Color side, MoveList& moveList) {
    Bitboard bitboard, attacks;
    int source_square, target_square;

    bitboard = pieceBitboards[side][Knight]; 

    while (bitboard) {

        source_square = getLSBIndex(bitboard);
        attacks = knightAttacks[source_square] & ~occupancyBitboards[side];

        while (attacks) {

            target_square = getLSBIndex(attacks);
            // quiet move
            if (!getBit(occupancyBitboards[!side], static_cast<Square>(target_square))) {
                moveList.add(encodeMove(source_square, target_square, side, Knight, 0, false, false, false, false));
            }
            else {
                // captures
                moveList.add(encodeMove(source_square, target_square, side, Knight, 0, true, false, false, false));
            }
            clearBit(attacks, static_cast<Square>(target_square));
        }
        clearBit(bitboard, static_cast<Square>(source_square));
    }
}

void bishopMoves(Color side, MoveList& moveList) {
    Bitboard bitboard, attacks;
    int source_square, target_square;

    bitboard = pieceBitboards[side][Bishop];

    while (bitboard) {

        source_square = getLSBIndex(bitboard);
        attacks = getBishopAttacks(source_square, occupancyBitboards[All]) & ~occupancyBitboards[side];

        while (attacks) {

            target_square = getLSBIndex(attacks);
            // quiet move
            if (!getBit(occupancyBitboards[!side], static_cast<Square>(target_square))) {
                moveList.add(encodeMove(source_square, target_square, side, Bishop, 0, false, false, false, false));
            }
            else {
                // captures
                moveList.add(encodeMove(source_square, target_square, side, Bishop, 0, true, false, false, false));
            }
            clearBit(attacks, static_cast<Square>(target_square));
        }
        clearBit(bitboard, static_cast<Square>(source_square));
    }
}

void rookMoves(Color side, MoveList& moveList) {
    Bitboard bitboard, attacks;
    int source_square, target_square;

    bitboard = pieceBitboards[side][Rook];

    while (bitboard) {

        source_square = getLSBIndex(bitboard);
        attacks = getRookAttacks(source_square, occupancyBitboards[All]) & ~occupancyBitboards[side];

        while (attacks) {

            target_square = getLSBIndex(attacks);
            // quiet move
            if (!getBit(occupancyBitboards[!side], static_cast<Square>(target_square))) {
                moveList.add(encodeMove(source_square, target_square, side, Rook, 0, false, false, false, false));
            }
            else {
                // captures
                moveList.add(encodeMove(source_square, target_square, side, Rook, 0, true, false, false, false));
            }
            clearBit(attacks, static_cast<Square>(target_square));
        }
        clearBit(bitboard, static_cast<Square>(source_square));
    }
}

void queenMoves(Color side, MoveList& moveList) {
    Bitboard bitboard, attacks;
    int source_square, target_square;

    bitboard = pieceBitboards[side][Queen];

    while (bitboard) {

        source_square = getLSBIndex(bitboard);
        attacks = getQueenAttacks(source_square, occupancyBitboards[All]) & ~occupancyBitboards[side];

        while (attacks) {

            target_square = getLSBIndex(attacks);
            // quiet move
            if (!getBit(occupancyBitboards[!side], static_cast<Square>(target_square))) {
                moveList.add(encodeMove(source_square, target_square, side, Queen, 0, false, false, false, false));
            }
            else {
                // captures
                moveList.add(encodeMove(source_square, target_square, side, Queen, 0, true, false, false, false));
            }
            clearBit(attacks, static_cast<Square>(target_square));
        }
        clearBit(bitboard, static_cast<Square>(source_square));
    }
}

MoveList generateMoves() {

    MoveList moves;
    pawnMoves(static_cast<Color>(side), moves);
    knightMoves(static_cast<Color>(side), moves);
    bishopMoves(static_cast<Color>(side), moves);
    rookMoves(static_cast<Color>(side), moves);
    queenMoves(static_cast<Color>(side), moves);
    kingMoves(static_cast<Color>(side), moves);
    return moves;
}

bool makeMove(Move move, MoveMode mode) {
    //preserve board state
    // Bitboard prev_pieceBitboards[2][6];
    // Bitboard prev_occupancyBitboards[3];
    // int prev_side, prev_enpassant, prev_castling;

    // parse move
    MoveStore m(move);

    if (mode == CAPTURES_ONLY) {
        if (!m.isCapture()) {
            return false;
        }
        else {
            return makeMove(move, ALL_MOVES);
        }
    }
    else {

        // Save current state into previous state
        // memcpy(prev_pieceBitboards, pieceBitboards, sizeof(pieceBitboards));
        // memcpy(prev_occupancyBitboards, occupancyBitboards, sizeof(occupancyBitboards));

        // prev_side = side;
        // prev_enpassant = enpassant;
        // prev_castling = castling;
        saveState();
        // make move
        clearBit(pieceBitboards[m.getColor()][m.getPiece()], static_cast<Square>(m.getSource()));
        setBit(pieceBitboards[m.getColor()][m.getPiece()], static_cast<Square>(m.getTarget()));

        // handling capture moves
        if (m.isCapture()) {
            // loop over bitboards to find which piece is being captured
            for (int piece = Pawn; piece <= King; piece++) {
                // if there's an opposing piece on the target square
                if (getBit(pieceBitboards[!m.getColor()][piece], static_cast<Square>(m.getTarget()))) {
                    // remove it from corresponding bitboard
                    clearBit(pieceBitboards[!m.getColor()][piece], static_cast<Square>(m.getTarget()));
                    break;
                }
            }
        }

        if (m.getPromoted()) {
            // erase the pawn from the target square
            clearBit(pieceBitboards[m.getColor()][Pawn], static_cast<Square>(m.getTarget()));

            // set up promoted piece on chess board
            setBit(pieceBitboards[m.getColor()][m.getPromoted()], static_cast<Square>(m.getTarget()));
        }
        
        if (m.isEnPassant()) {
            int square_offset = (m.getColor() == White) ? -8 : 8;
            clearBit(pieceBitboards[!m.getColor()][Pawn], static_cast<Square>(m.getTarget() + square_offset));
        }
        enpassant = no_sq;

        if (m.isDoublePush()) {
            int square_offset = (m.getColor() == White) ? -8 : 8;
            enpassant = m.getTarget() + square_offset;
        }

        if (m.isCastling()) {
            switch (m.getTarget())
            {
                // white castles king side
            case (g1):
                // move H rook
                clearBit(pieceBitboards[White][Rook], h1);
                setBit(pieceBitboards[White][Rook], f1);
                break;

                // white castles queen side
            case (c1):
                // move A rook
                clearBit(pieceBitboards[White][Rook], a1);
                setBit(pieceBitboards[White][Rook], d1);
                break;

                // black castles king side
            case (g8):
                // move H rook
                clearBit(pieceBitboards[Black][Rook], h8);
                setBit(pieceBitboards[Black][Rook], f8);
                break;

                // black castles queen side
            case (c8):
                // move A rook
                clearBit(pieceBitboards[Black][Rook], a8);
                setBit(pieceBitboards[Black][Rook], d8);
                break;
            }
        }
        // update castling rights
        castling &= castling_rights[m.getSource()];
        castling &= castling_rights[m.getTarget()];
        
        // Set occupancy boards
        memset(occupancyBitboards, 0ULL, sizeof(occupancyBitboards));
        for (int color = White; color <= Black; color++) {
            for (int piece = Pawn; piece <= King; piece++) {
                occupancyBitboards[color] |= pieceBitboards[color][piece];
            }
        }
        occupancyBitboards[All] |= occupancyBitboards[White];
        occupancyBitboards[All] |= occupancyBitboards[Black];
        
        // std::cout << side << " made move: " << std::endl;
        // change side
        side ^= 1;
        // std::cout << side << " changed to : " << std::endl;
        // make sure king of current side is not being attacked by the other side after this side's move
        if (isSquareAttacked(static_cast<Square>(getLSBIndex(pieceBitboards[!side][King])), static_cast<Color>(side))) {
            // take move back
            // std::cout << !side << " king attacked by " << side << " after move!" << std::endl;
            // Restore previous state
            // memcpy(pieceBitboards, prev_pieceBitboards, sizeof(pieceBitboards));
            // memcpy(occupancyBitboards, prev_occupancyBitboards, sizeof(occupancyBitboards));

            // side = prev_side;
            // enpassant = prev_enpassant;
            // castling = prev_castling;
            takeBack();
            // return illegal move
            return false;
        }
        else {

            return true;
        }
    }
};

// const MoveList& getMoveList() const {
//     return moves;
// }

MoveStore::MoveStore(Move move)
    : source(move& FROM_SQ_MASK),
    target((move& TO_SQ_MASK) >> 6),
    color((move& COLOR_MASK) >> 12),
    piece((move& PIECE_MASK) >> 13),
    promoted((move& PROMO_MASK) >> 16),
    capture(move& CAPTURE_FLAG),
    doubleM(move& DOUBLE_FLAG),
    enpassant(move& ENPASSANT_FLAG),
    castling(move& CASTLE_FLAG)
{
}

int MoveStore::getSource() const { return  source; }
int MoveStore::getTarget() const { return target; }
int MoveStore::getColor() const { return color; }
int MoveStore::getPiece() const { return piece; }
int MoveStore::getPromoted() const { return promoted; }

bool MoveStore::isCapture() const { return capture; }
bool MoveStore::isDoublePush() const { return doubleM; }
bool MoveStore::isEnPassant() const { return enpassant; }
bool MoveStore::isCastling() const { return castling; }

// helper methods // 
void printMove(Move move) {
    std::cout << std::left
    << "    "  // indent
    << std::setw(10) << "Move"
    << std::setw(10) << "Piece"
    << std::setw(10) << "Capture"
    << std::setw(10) << "Double"
    << std::setw(10) << "EnPass"
    << std::setw(10) << "Castling"
    << '\n';
    MoveStore m(move);

    // const std::string& from = SquareNames[m.getSource()];
    // const std::string& to = SquareNames[m.getTarget()];
    // char promo = PromotedPieces[m.getPromoted()];

    // std::cout << from << to;
    // if (promo != ' ') std::cout << promo;
    // std::cout << '\n';
    const std::string& from = SquareNames[m.getSource()];
    const std::string& to = SquareNames[m.getTarget()];
    const std::string& promo = PromotedPieces[m.getPromoted()];
    std::string piece = PieceSymbols[m.getColor()][m.getPiece()];

    std::cout << "    "
        << std::left << std::setw(10) << (from + to + (promo != " " ? promo : " "))
        << std::setw(10) << piece
        << std::setw(10) << m.isCapture()
        << std::setw(10) << m.isDoublePush()
        << std::setw(10) << m.isEnPassant()
        << std::setw(10) << m.isCastling()
        << '\n';
}

void printMoves(const MoveList& moves) {
    std::cout << std::left
        << "    "  // indent
        << std::setw(10) << "Move"
        << std::setw(10) << "Piece"
        << std::setw(10) << "Capture"
        << std::setw(10) << "Double"
        << std::setw(10) << "EnPass"
        << std::setw(10) << "Castling"
        << '\n';

    // Loop over moves in the list
    for (size_t i = 0; i < moves.size(); ++i)
    {
        const Move move = moves[i];
        MoveStore m(move);

        const std::string& from = SquareNames[m.getSource()];
        const std::string& to = SquareNames[m.getTarget()];
        const std::string& promo = PromotedPieces[m.getPromoted()];
        std::string piece = PieceSymbols[m.getColor()][m.getPiece()];
        std::cout << m.getPromoted() << std::endl;
        std::cout << "    "
            << std::left << std::setw(10) << (from + to + (promo != " " ? promo : " "))
            << std::setw(10) << piece
            << std::setw(10) << m.isCapture()
            << std::setw(10) << m.isDoublePush()
            << std::setw(10) << m.isEnPassant()
            << std::setw(10) << m.isCastling()
            << '\n';
    }

    std::cout << "\nTotal number of moves: " << moves.size() << "\n\n";
}

void printAttackedSquares(Color side) {
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << rank + 1 << "   ";
        for (int file = 0; file < 8; ++file) {
            int square = rank * 8 + file;
            std::cout << (isSquareAttacked(static_cast<Square>(square), side) ? "1 " : ". ");
        }
        std::cout << "\n";
    }
    std::cout << "\n";
    std::cout << "    a b c d e f g h\n";
}

void printBoard() {
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << rank + 1 << "   ";
        for (int file = 0; file < 8; ++file) {
            int square = rank * 8 + file;

            int piece_index = -1;
            int piece_color = -1;
            for (int color = White; color <= Black; color++) {
                for (int piece = Pawn; piece <= King; piece++) {
                    if (getBit(pieceBitboards[color][piece], static_cast<Square>(square))) {
                        piece_index = piece;
                        piece_color = color;
                        break;
                    }
                }
            }
            std::cout << (piece_index == -1 ? ". " : std::string(PieceSymbols[piece_color][piece_index]) + " ");
        }
        std::cout << "\n";
    }
    std::cout << "\n";
    std::cout << "    a b c d e f g h\n";

    std::cout << "  Side:     " << (!side ? "white" : "black") << std::endl;;
    // print enpassant square
    std::cout << "  Enpassant:   " << (enpassant != no_sq ? SquareNames[enpassant] : "no") << std::endl;

    // print castling rights
    std::cout << "  Castling:  " << 
        (castling & wk ? 'K' : '-') << 
        (castling & wq ? 'Q' : '-') << 
        (castling & bk ? 'k' : '-') << 
        (castling & bq ? 'q' : '-') << std::endl;

}

// Example: create a move
Move encodeMove(int source, int target, int color, int piece, int promoted, bool capture = false,
    bool doubleM = false, bool enpassant = false, bool castling = false) {

    Move m = 0;
    m |= source;
    m |= (target << 6);
    m |= (color << 12);
    m |= (piece << 13);
    m |= (promoted << 16);

    if (capture) m |= CAPTURE_FLAG;
    if (doubleM) m |= DOUBLE_FLAG;
    if (enpassant) m |= ENPASSANT_FLAG;
    if (castling) m |= CASTLE_FLAG;

    return m;
}

void printBitboard(Bitboard bb) {
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << rank + 1 << "   ";
        for (int file = 0; file < 8; ++file) {
            int square = rank * 8 + file;
            std::cout << (getBit(bb, static_cast<Square>(square)) ? "1 " : ". ");
        }
        std::cout << "\n";
    }
    std::cout << "\n";
    std::cout << "    a b c d e f g h\n";

    // Print the bitboard as an unsigned 64-bit integer
    std::cout << "\nBitboard value: " << static_cast<uint64_t>(bb) << "\n\n";
}

void printPieceboards() {
    // print out initial boards
    for (int color = White; color <= Black; color++) {
        for (int piece = Pawn; piece <= King; piece++) {
            Bitboard bitboard = pieceBitboards[color][piece];
            std::cout << ColorNames[color] << " " << PieceTypeNames[piece] << " has bitboard: " << "\n";
            printBitboard(bitboard);
        }
    }
}

void printOccupancyboards() {
    for (int color = White; color <= All; color++) {
        Bitboard bitboard = occupancyBitboards[color];
        std::cout << ColorNames[color] << " " << " has occupancy bitboard: " << "\n";
        printBitboard(bitboard);
    }
}

// MoveList (fixed-size array implementation)
MoveList::MoveList() {
    count = 0;
}

void MoveList::add(Move move) noexcept {
    if (count < 256) {
        moves[count++] = move;
    }
}

void MoveList::clear() noexcept {
    count = 0;
}

size_t MoveList::size() const noexcept {
    return count;
}

bool MoveList::empty() const noexcept {
    return count == 0;
}

Move MoveList::operator[](size_t i) const noexcept {
    return moves[i];
}

Move& MoveList::operator[](size_t i) noexcept {
    return moves[i];
}

// get time in milliseconds
uint64_t get_time_ms()
{
    return GetTickCount64();
}

uint64_t perft(int depth)
{
    if (depth == 0)
        return 1ULL;

    uint64_t nodes = 0ULL;
 
    // generate moves for this board state
    MoveList moveList = generateMoves();
    // printBoard();
    // printMoves(moveList);

    for (size_t i = 0; i < moveList.size(); i++) {
        Move move = moveList[i];
        
        saveState();
        // makeMove mutates the board directly
        if (!makeMove(move, MoveMode::ALL_MOVES))
            continue;
  
        // recurse with mutated board
        nodes += perft(depth - 1);
        // board is restored automatically because we pass by value
        takeBack();
    }

    return nodes;
}

int main()
{   
    std::cout << "initializing tables" << std::endl;
    // initialize piece bitboards to 0  
    initTables();
    // initialize attack tables for leaper pieces (Pawn, Knight, King)
    initLeaperPieces();
    // initialize attack tables for sliding pieces (Bishop, Rook, Queen)
    initSliderPieces();
    std::cout << "\n";

    parseFEN(start_position);
    int start = get_time_ms();
    uint64_t nodes = perft(5);
    std::cout << "time taken to execute: " << get_time_ms() - start << std::endl;
    std::cout << "nodes: " << nodes << "\n";
    

    // parseFEN("8/8/8/8/8/8/8/8 w KQkq e6 0 1");
    // parseFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1 ");
    // MoveList movesList = generateMoves();
    // printMoves(movesList);
    
    // std::cout << "number of moves: " << movesList.size() << std::endl;
    // for (size_t i = 0; i < movesList.size(); i++) {

    //     Move move = movesList[i];
    //     saveState();

    //     if (!makeMove(move, ALL_MOVES)) {
    //         std::cout << "illegal move" << std::endl;
    //         continue;
    //     }
    //     printMove(move);
    //     printBoard();
    //     getchar();
    //     takeBack();
    //     printBoard();
    //     getchar();
    // }

    return 0;
}

