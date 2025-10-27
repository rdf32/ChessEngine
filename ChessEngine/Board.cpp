
#include <cstdint>
#include <iostream>
#include "Board.h"

//Bit index : Square: bigger number left << smaller >> right
//56 57 58 59 60 61 62 63 ->a8 to h8
//48 49 50 51 52 53 54 55 ->a7 to h7
//40 41 42 43 44 45 46 47 ->a6 to h6
//32 33 34 35 36 37 38 39 ->a5 to h5
//24 25 26 27 28 29 30 31 ->a4 to h4
//16 17 18 19 20 21 22 23 ->a3 to h3
//8  9  10 11 12 13 14 15 ->a2 to h2
//0  1  2  3  4  5  6  7  ->a1 to h1

const char* Board::ColorNames[3] = { "White", "Black", "All" };
const char* Board::PieceTypeNames[6] = { "Pawn", "Knight", "Bishop", "Rook", "Queen", "King" };
const char* Board::SquareNames[64] = {
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
    };

void Board::setBit(Bitboard& bitboard, Square square) {
    bitboard |= (1ULL << square);
}

void Board::clearBit(Bitboard& bitboard, Square square) {
    bitboard &= ~(1ULL << square);
}

bool Board::getBit(Bitboard bitboard, Square square) {
    return bitboard & (1ULL << square);
}

int Board::countBits(Bitboard bitboard) {

    int count = 0;

    while (bitboard) {
        bitboard &= bitboard - 1;
        count++;
    }
    return count;
}

int Board::getLSBIndex(Bitboard bitboard) {

    if (bitboard) {
        return countBits((bitboard & (~bitboard + 1)) - 1);
    }
    else {
        return -1;
    }
}

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
Board::Bitboard get_random_U64_number() {
    // define 4 random numbers
    Board::Bitboard n1, n2, n3, n4;

    // init random numbers slicing 16 bits from MS1B side
    n1 = (Board::Bitboard)(get_random_U32_number()) & 0xFFFF;
    n2 = (Board::Bitboard)(get_random_U32_number()) & 0xFFFF;
    n3 = (Board::Bitboard)(get_random_U32_number()) & 0xFFFF;
    n4 = (Board::Bitboard)(get_random_U32_number()) & 0xFFFF;

    // return random number
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

// generate magic number candidate
Board::Bitboard generate_magic_number() {
    return get_random_U64_number() & get_random_U64_number() & get_random_U64_number();
}

Board::Board() {   
    // initialize piece bitboards to 0  
    initTables();
    // set the starting board position
    setStartingPosition();
    // initialize attack tables for leaper pieces (Pawn, Knight, King)
    initLeaperPieces();
    // initialize attack tables for sliding pieces (Bishop, Rook, Queen)
    //initSlidingPieces();
}

Board::Bitboard Board::maskPawnAttacks(Color color, Square square) const {

    Bitboard bitboard = 0ULL;
    Bitboard attacks = 0ULL;

    setBit(bitboard, square);
    if (color == White) {

        if ((bitboard << 7) & Board::notFile_H) { attacks |= (bitboard << 7); }
        if ((bitboard << 9) & Board::notFile_A) { attacks |= (bitboard << 9); }

    }
    else {
        if ((bitboard >> 7) & Board::notFile_A) { attacks |= (bitboard >> 7); }
        if ((bitboard >> 9) & Board::notFile_H) { attacks |= (bitboard >> 9); }
    }

    return attacks;
}

Board::Bitboard Board::maskKnightAttacks(Square square) const {

    Bitboard bitboard = 0ULL;
    Bitboard attacks = 0ULL;

    setBit(bitboard, square);
    if ((bitboard << 6) & Board::notFile_HG) { attacks |= (bitboard << 6); }   // 2 files left (spatially)
    if ((bitboard << 15) & Board::notFile_H) { attacks |= (bitboard << 15); }  // 1 file left (spatially)
    if ((bitboard << 17) & Board::notFile_A) { attacks |= (bitboard << 17); }  // 1 file right (spatially)
    if ((bitboard << 10) & Board::notFile_AB) { attacks |= (bitboard << 10); } // 2 files right (spatially)

    if ((bitboard >> 6) & Board::notFile_AB) { attacks |= (bitboard >> 6); }   // 2 files right (spatially)
    if ((bitboard >> 15) & Board::notFile_A) { attacks |= (bitboard >> 15); }  // 1 file right (spatially)
    if ((bitboard >> 17) & Board::notFile_H) { attacks |= (bitboard >> 17); }  // 1 file left (spatially)
    if ((bitboard >> 10) & Board::notFile_HG) { attacks |= (bitboard >> 10); } // 2 files left (spatially)

    return attacks;
}

Board::Bitboard Board::maskKingAttacks(Square square) const {

    Bitboard bitboard = 0ULL;
    Bitboard attacks = 0ULL;

    setBit(bitboard, square);
    if ((bitboard << 7) & Board::notFile_H) { attacks |= (bitboard << 7); }
    if ((bitboard << 9) & Board::notFile_A) { attacks |= (bitboard << 9); }
    if ((bitboard >> 7) & Board::notFile_A) { attacks |= (bitboard >> 7); }
    if ((bitboard >> 9) & Board::notFile_H) { attacks |= (bitboard >> 9); }

    if (bitboard << 8) { attacks |= (bitboard << 8); }
    if (bitboard >> 8) { attacks |= (bitboard >> 8); }
    if ((bitboard << 1) & Board::notFile_A) { attacks |= (bitboard << 1); }
    if ((bitboard >> 1) & Board::notFile_H) { attacks |= (bitboard >> 1); }

    return attacks;
}

Board::Bitboard Board::maskBishopAttacks(Square square) const {

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

Board::Bitboard Board::maskRookAttacks(Square square) const {

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

Board::Bitboard Board::dynamicBishopAttacks(Square square, Bitboard blocker) const {

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

Board::Bitboard Board::dynamicRookAttacks(Square square, Bitboard blocker) const {

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

Board::Bitboard Board::setOccupancy(int index, int numMaskBits, Bitboard attackMask) const {

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

// find appropriate magic number
Board::Bitboard Board::find_magic_number(int square, int relevant_bits, int bishop) {
    // init occupancies
    Board::Bitboard occupancies[4096];

    // init attack tables
    Board::Bitboard attacks[4096];

    // init used attacks
    Board::Bitboard used_attacks[4096];

    // init attack mask for a current piece
    Board::Bitboard attack_mask = bishop ? maskBishopAttacks(static_cast<Square>(square)) : maskRookAttacks(static_cast<Square>(square));

    // init occupancy indicies
    int occupancy_indicies = 1 << relevant_bits;

    // loop over occupancy indicies
    for (int index = 0; index < occupancy_indicies; index++)
    {
        // init occupancies
        occupancies[index] = setOccupancy(index, relevant_bits, attack_mask);

        // init attacks
        attacks[index] = bishop ? dynamicBishopAttacks(static_cast<Square>(square), occupancies[index]) :
            dynamicRookAttacks(static_cast<Square>(square), occupancies[index]);
    }

    // test magic numbers loop
    for (int random_count = 0; random_count < 100000000; random_count++)
    {
        // generate magic number candidate
        Board::Bitboard magic_number = generate_magic_number();

        // skip inappropriate magic numbers
        if (countBits((attack_mask * magic_number) & 0xFF00000000000000) < 6) continue;

        // init used attacks
        memset(used_attacks, 0ULL, sizeof(used_attacks));

        // init index & fail flag
        int index, fail;

        // test magic index loop
        for (index = 0, fail = 0; !fail && index < occupancy_indicies; index++)
        {
            // init magic index
            int magic_index = (int)((occupancies[index] * magic_number) >> (64 - relevant_bits));

            // if magic index works
            if (used_attacks[magic_index] == 0ULL)
                // init used attacks
                used_attacks[magic_index] = attacks[index];

            // otherwise
            else if (used_attacks[magic_index] != attacks[index])
                // magic index doesn't work
                fail = 1;
        }

        // if magic number works
        if (!fail)
            // return it
            return magic_number;
    }

    // if magic number doesn't work
    printf("  Magic number fails!\n");
    return 0ULL;
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

void Board::initLeaperPieces() {
    for (int square = a1; square <= h8; square++) {

        // initialize pawn attacks pawnAttacks[color][square]
        pawnAttacks[White][square] = maskPawnAttacks(White, static_cast<Square>(square));
        pawnAttacks[Black][square] = maskPawnAttacks(Black, static_cast<Square>(square));

        // inititalize knight attacks knightAttacks[square]
        knightAttacks[square] = maskKnightAttacks(static_cast<Square>(square));

        // inititalize king attacks kingAttacks[square]
        kingAttacks[square] = maskKingAttacks(static_cast<Square>(square));
    }
}

// init magic numbers
void Board::init_magic_numbers() {
    // loop over 64 board squares
    for (int square = 0; square < 64; square++) {
        // init rook magic numbers
        //Square square = static_cast<Square>((rank * 8 + file));
        rook_magic_numbers[square] = find_magic_number(square, relevantBitcountRook[square], 0);
        printf(" 0x%llxULL\n", rook_magic_numbers[square]);
    }
    printf("\n");
    printf("\n");
    // loop over 64 board squares
    for (int square = 0; square < 64; square++) {
        // init bishop magic numbers
        //Square square = static_cast<Square>((rank * 8 + file));
        bishop_magic_numbers[square] = find_magic_number(square, relevantBitcountBishop[square], 1);
        printf(" 0x%llxULL\n", bishop_magic_numbers[square]);
    }
}

// helper methods // 
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





