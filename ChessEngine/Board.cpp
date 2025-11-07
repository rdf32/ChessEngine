#pragma once
#include <cstdint>
#include <iostream>
#include <string>
#include <sstream>
#include <array>

#include "Board.h"
#include "Logger.h"

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
        bitboard &= bitboard - 1;
        count++;
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

Board::Board() {   
    // initialize piece bitboards to 0  
    initTables();
    // set the starting board position
    setStartingPosition();
    // initialize attack tables for leaper pieces (Pawn, Knight, King)
    initLeaperPieces();
    // initialize attack tables for sliding pieces (Bishop, Rook, Queen)
    initSliderPieces();

    side = White;
    enpassant = no_sq;
    castle |= wk;
    castle |= wq;
    castle |= bk;
    castle |= bq;

    printBoard();
}

Bitboard Board::maskPawnAttacks(Color color, Square square) const {

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

Bitboard Board::maskKnightAttacks(Square square) const {

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

Bitboard Board::maskKingAttacks(Square square) const {

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

Bitboard Board::maskBishopAttacks(Square square) const {

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

Bitboard Board::maskRookAttacks(Square square) const {

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

Bitboard Board::dynamicBishopAttacks(Square square, Bitboard blocker) const {

    Bitboard attacks = 0ULL;

    int rank, file;
    int target_rank = square / 8;
    int target_file = square % 8;

    for (rank = target_rank + 1, file = target_file + 1; rank <= 7 && file <= 7; rank++, file++) { 
        setBit(attacks, static_cast<Square>(rank * 8 + file));
        if (1ULL << (rank * 8 + file) & blocker) { break; }
    }
    for (rank = target_rank + 1, file = target_file - 1; rank <= 7 && file >= 0; rank++, file--) { 
        setBit(attacks, static_cast<Square>(rank * 8 + file));
        if (1ULL << (rank * 8 + file) & blocker) { break; }
    }
    for (rank = target_rank - 1, file = target_file + 1; rank >= 0 && file <= 7; rank--, file++) { 
        setBit(attacks, static_cast<Square>(rank * 8 + file));
        if (1ULL << (rank * 8 + file) & blocker) { break; }
    }
    for (rank = target_rank - 1, file = target_file - 1; rank >= 0 && file >= 0; rank--, file--) { 
        setBit(attacks, static_cast<Square>(rank * 8 + file));
        if (1ULL << (rank * 8 + file) & blocker) { break; } // these could be getBits calls instead of written out
    }

    return attacks;
}

Bitboard Board::dynamicRookAttacks(Square square, Bitboard blocker) const {

    Bitboard attacks = 0ULL;

    int rank, file;
    int target_rank = square / 8;
    int target_file = square % 8;

    for (rank = target_rank + 1; rank <= 7; rank++) { 
        setBit(attacks, static_cast<Square>(rank * 8 + target_file));
        if (1ULL << (rank * 8 + target_file) & blocker) { break; }
    }
    for (rank = target_rank - 1; rank >= 0; rank--) { 
        setBit(attacks, static_cast<Square>(rank * 8 + target_file));
        if (1ULL << (rank * 8 + target_file) & blocker) { break; }
    }
    for (file = target_file + 1; file <= 7; file++) { 
        setBit(attacks, static_cast<Square>(target_rank * 8 + file));
        if (1ULL << (target_rank * 8 + file) & blocker) { break; }
    }
    for (file = target_file - 1; file >= 0; file--) { 
        setBit(attacks, static_cast<Square>(target_rank * 8 + file));
        if (1ULL << (target_rank * 8 + file) & blocker) { break; }
    }

    return attacks;
}

// initialization methods //
void Board::initTables() {
    for (int piece = Pawn; piece <= King; piece++) {
        pieceBitboards[White][piece] = 0ULL;
        pieceBitboards[Black][piece] = 0ULL;
    }
    // initialize occupancy bitboards to 0
    for (int color = White; color <= All; color++) {
        occupancyBitboards[color] = 0ULL;
    }
}

void Board::setStartingPosition() {

    // set up white pieces for starting position
    pieceBitboards[White][Pawn] = 65280ULL;
    pieceBitboards[White][Knight] = 66ULL;
    pieceBitboards[White][Bishop] = 36ULL;
    pieceBitboards[White][Rook] = 129ULL;
    pieceBitboards[White][Queen] = 8ULL;
    pieceBitboards[White][King] = 16ULL;

    // set up black pieces for starting position
    pieceBitboards[Black][Pawn] = 71776119061217280ULL;
    pieceBitboards[Black][Knight] = 4755801206503243776ULL;
    pieceBitboards[Black][Bishop] = 2594073385365405696ULL;
    pieceBitboards[Black][Rook] = 9295429630892703744ULL;
    pieceBitboards[Black][Queen] = 576460752303423488ULL;
    pieceBitboards[Black][King] = 1152921504606846976ULL;

    // set up occupancy boards for starting position
    occupancyBitboards[White] = 65535ULL;
    occupancyBitboards[Black] = 18446462598732840960ULL;
    occupancyBitboards[All] = occupancyBitboards[White] | occupancyBitboards[Black];

}

void Board::initLeaperPieces() const {
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

void Board::initSliderPieces() const {

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

            //std::cout << "Occupancy Inds: " << occupancyInds << std::endl;
            //printBitboard(static_cast<Bitboard>(occupancyInds));

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
inline Bitboard Board::getBishopAttacks(int square, Bitboard occupancy) const {
    // get bishop attacks assuming current board occupancy
    occupancy &= bishop_masks[square];
    occupancy *= bishop_magic_numbers[square];
    occupancy >>= 64 - relevantBitcountBishop[square];

    // return bishop attacks
    return bishop_attacks[square][occupancy];
}

// get rook attacks
inline Bitboard Board::getRookAttacks(int square, Bitboard occupancy) const {
    // get bishop attacks assuming current board occupancy
    occupancy &= rook_masks[square];
    occupancy *= rook_magic_numbers[square];
    occupancy >>= 64 - relevantBitcountRook[square];

    // return rook attacks
    return rook_attacks[square][occupancy];
}


void Board::parseFEN(const std::string& fen) {

    side = White;
    enpassant = no_sq;
    castle = 0;

    memset(pieceBitboards, 0, sizeof(pieceBitboards));
    memset(occupancyBitboards, 0, sizeof(occupancyBitboards));

    std::string boardT, sideT, castleT, enpassantT;
    int halfmovelock, fullmovenumber;

    std::istringstream ss(fen);
    ss >> boardT >> sideT >> castleT >> enpassantT >> halfmovelock >> fullmovenumber;

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
        case 'K': castle |= wk; break;
        case 'Q': castle |= wq; break;
        case 'k': castle |= bk; break;
        case 'q': castle |= bq; break;
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
    occupancyBitboards[All] = occupancyBitboards[White] | occupancyBitboards[Black];

    //printBoard();

    //for (int color = White; color <= All; color++) {
        //printBitboard(occupancyBitboards[color]);
    //}
}

// helper methods // 

void Board::printBoard() const {
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
        (castle & wk ? 'K' : '-') << 
        (castle & wq ? 'Q' : '-') << 
        (castle & bk ? 'k' : '-') << 
        (castle & bq ? 'q' : '-') << std::endl;

}


void Board::printBitboard(Bitboard bb) {
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

void Board::printPieceboards() {
    // print out initial boards
    for (int color = White; color <= Black; color++) {
        for (int piece = Pawn; piece <= King; piece++) {
            Bitboard bitboard = pieceBitboards[color][piece];
            std::cout << ColorNames[color] << " " << PieceTypeNames[piece] << " has bitboard: " << "\n";
            printBitboard(bitboard);
        }
    }
}

void Board::printOccupancyboards() {
    for (int color = White; color <= All; color++) {
        Bitboard bitboard = occupancyBitboards[color];
        std::cout << ColorNames[color] << " " << " has occupancy bitboard: " << "\n";
        printBitboard(bitboard);
    }
}

// find appropriate magic number
//Board::Bitboard Board::findMagicNumber(int square, int relevant_bits, int bishop) {
//    // init occupancies
//    Board::Bitboard occupancies[4096];
//
//    // init attack tables
//    Board::Bitboard attacks[4096];
//
//    // init used attacks
//    Board::Bitboard used_attacks[4096];
//
//    // init attack mask for a current piece
//    Board::Bitboard attack_mask = bishop ? maskBishopAttacks(static_cast<Square>(square)) : maskRookAttacks(static_cast<Square>(square));
//
//    // init occupancy indicies
//    int occupancy_indicies = 1 << relevant_bits;
//
//    // loop over occupancy indicies
//    for (int index = 0; index < occupancy_indicies; index++)
//    {
//        // init occupancies
//        occupancies[index] = setOccupancy(index, relevant_bits, attack_mask);
//
//        // init attacks
//        attacks[index] = bishop ? dynamicBishopAttacks(static_cast<Square>(square), occupancies[index]) :
//            dynamicRookAttacks(static_cast<Square>(square), occupancies[index]);
//    }
//
//    // test magic numbers loop
//    for (int random_count = 0; random_count < 100000000; random_count++)
//    {
//        // generate magic number candidate
//        Board::Bitboard magic_number = generate_magic_number();
//
//        // skip inappropriate magic numbers
//        if (countBits((attack_mask * magic_number) & 0xFF00000000000000) < 6) continue;
//
//        // init used attacks
//        memset(used_attacks, 0ULL, sizeof(used_attacks));
//
//        // init index & fail flag
//        int index, fail;
//
//        // test magic index loop
//        for (index = 0, fail = 0; !fail && index < occupancy_indicies; index++)
//        {
//            // init magic index
//            int magic_index = (int)((occupancies[index] * magic_number) >> (64 - relevant_bits));
//
//            // if magic index works
//            if (used_attacks[magic_index] == 0ULL)
//                // init used attacks
//                used_attacks[magic_index] = attacks[index];
//
//            // otherwise
//            else if (used_attacks[magic_index] != attacks[index])
//                // magic index doesn't work
//                fail = 1;
//        }
//
//        // if magic number works
//        if (!fail)
//            // return it
//            return magic_number;
//    }
//
//    // if magic number doesn't work
//    printf("  Magic number fails!\n");
//    return 0ULL;
//}
//
//// init magic numbers
//void Board::initMagicNumbers() {
//    // loop over 64 board squares
//    for (int square = 0; square < 64; square++) {
//        // init rook magic numbers
//        //Square square = static_cast<Square>((rank * 8 + file));
//        //rook_magic_numbers[square] = findMagicNumber(square, relevantBitcountRook[square], 0);
//        printf(" 0x%llxULL,\n", rook_magic_numbers[square]);
//    }
//    printf("\n");
//    printf("\n");
//    // loop over 64 board squares
//    for (int square = 0; square < 64; square++) {
//        // init bishop magic numbers
//        //Square square = static_cast<Square>((rank * 8 + file));
//        //bishop_magic_numbers[square] = findMagicNumber(square, relevantBitcountBishop[square], 1);
//        printf(" 0x%llxULL,\n", bishop_magic_numbers[square]);
//    }
//}




