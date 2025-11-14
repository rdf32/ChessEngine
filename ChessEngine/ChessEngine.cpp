// ChessEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.
#pragma once
#include <iostream>
#include "Board.h"

// FEN dedug positions
#define empty_board "8/8/8/8/8/8/8/8 w - - "
#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define killer_position "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmk_position "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "


int main()
{   
    std::cout << "creating board 1" << std::endl;
    Board board;
    std::cout << "\n";

    //board.addMove(encodeMove(d7, e8, White, Bishop, Queen, false, false, false, true));
    board.parseFEN("r3k2r/p1ppRpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R b KQkq - 0 1 ");
    //board.generateMoves();
    //board.printMoves();
    board.generateMoves();

    const MoveList& moves = board.getMoveList();
    std::cout << "number of moves: " << moves.size() << std::endl;
    for (size_t i = 0; i < moves.size(); i++) {

        Move move = moves[i];

        board.saveState();
        if (!board.makeMove(move, ALL_MOVES)) {
            continue;
        }
        board.printBoard();
        getchar();
        board.takeBack();
        board.printBoard();
        getchar();
    }
    //std::cout << "creating board 2" << std::endl;
    //Board board2;
    //std::cout << "\n";

    //std::cout << "creating board 3" << std::endl;
    //Board board3;
    //std::cout << "\n";
    
    //board.parseFEN(empty_board);
    //board.printAttackedSquares(White);
    //board.printAttackedSquares(Black);
    //std::cout << "\n";

    //board.parseFEN(start_position);
    //board.parseFEN("8/4P3/2P5/8/8/2p5/4p3/8 w - - ");
    //board.printAttackedSquares(White);
    //board.printAttackedSquares(Black);
    //board.pawnMoves(White);
    //board.pawnMoves(Black);

    //std::cout << "\n";

    //board.parseFEN(tricky_position);
    //board.printAttackedSquares(White);
    //board.printAttackedSquares(Black);
    //std::cout << "\n";

    //board.parseFEN(killer_position);
    //board.printAttackedSquares(White);
    //board.printAttackedSquares(Black);
    //board.generateMoves(White);
    //board.generateMoves(Black);
    //std::cout << "\n";

    //board.parseFEN(cmk_position);
    //board.printAttackedSquares(White);
    //board.printAttackedSquares(Black);

    //Bitboard bitboard = 0ULL;

    //setBit(bitboard, e5);
    //board.printBitboard(board.getQueenAttacks(e4, bitboard));

        // create move
  /*  encodeMove(int source, int target, int color, int piece, int pcolor, int promoted, bool capture = false,
        bool doubleM = false, bool enpassant = false, bool castling = false)*/
 /*   int move = encodeMove(d7, e8, White, Pawn, White, Queen, true, false, false, false);
    printMove(move);*/

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