// ChessEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.
#pragma once
#include <iostream>
#include "Board.h"

int main()
{   
    std::cout << "creating board 1" << std::endl;
    Board board;
    std::cout << "\n";

    std::cout << "creating board 2" << std::endl;
    Board board2;
    std::cout << "\n";

    std::cout << "creating board 3" << std::endl;
    Board board3;
    std::cout << "\n";


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