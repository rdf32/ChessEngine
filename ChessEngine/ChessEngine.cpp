// ChessEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include <iostream>
#include "Board.h"

int main()
{   

    Board board;
    board.printPieceboards();
    board.printOccupancyboards();

    //for (int square = Board::A1; square <= Board::H8; square++) {
    //    std::cout << "Square: " << square << std::endl;
    //    Board::Square target_square = static_cast<Board::Square>(square);
    //    //printBitboard(board.rookMask(target_square));
    //    //printBitboard(board.kingMask(target_square));
    //    //printBitboard(board.bishopAttacksOTF(target_square, 0ULL));
    //    //printBitboard(board.pawnMask(Board::White, target_square));

    //}
    //Board::Bitboard blocker = 0ULL;
    //Board::setBit(blocker, Board::b6);
    //Board::setBit(blocker, Board::g7);
    //Board::setBit(blocker, Board::e3);
    //Board::setBit(blocker, Board::b2);
    //printBitboard(blocker);
    //printBitboard(board.dynamicBishopAttacks(Board::d4, blocker));

    //Board::Bitboard blocker2 = 0ULL;
    //Board::setBit(blocker2, Board::d6);
    //Board::setBit(blocker2, Board::h4);
    //Board::setBit(blocker2, Board::d1);
    //Board::setBit(blocker2, Board::b4);
    //printBitboard(blocker2);
    //printBitboard(board.dynamicRookAttacks(Board::d4, blocker2));

    //printBitboard(blocker);
    //std::cout << board.countBits(blocker) << std::endl;
    //std::cout << board.getLSBIndex(blocker) << std::endl;

    //for (int rank = 0; rank < 8; rank++) {
    //    for (int file = 0; file < 8; file++) {
    //        int square = rank * 8 + file;
    //        std::cout << board.countBits(board.maskRookAttacks(static_cast<Board::Square>(square))) << ", ";
    //    }
    //    std::cout << std::endl;
    //}

    return 0;
}



// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file



// Next Step Suggestions
//
//Here’s what you could consider implementing next, in order :
//
//Bitboard Masks
//
//Precompute attack masks for knights, kings, and pawn moves.
//
//Add sliding attacks using magic bitboards or attack tables.
//
//Move Generation
//
//Generate pseudo - legal moves for each piece.
//
//Add legality checking(e.g.king safety).
//
//Move Encoding
//
//Use a 32 - bit integer for encoding moves with fields for from, to, promotion, etc.
//
//FEN Support
//
//Read and write board positions from Forsyth - Edwards Notation.
//
//Perft Testing
//
//Validate correctness of your move generator.
//
//UCI Protocol
//
//Hook up your engine to a GUI like Arena or CuteChess.