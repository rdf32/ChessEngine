import chess_engine



if __name__ == "__main__":

    fen = "rnb1kbnr/pppp1ppp/4p3/8/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3"

    board = chess_engine.Board()
    board.parse_fen(fen)

    state = board.get_state()
    legal_moves = board.legal_moves()

    print("FEN:", fen)
    print("Parsed Board State:")    
    print(state.in_check)


    print(legal_moves)

    if len(legal_moves) == 0:
        if state.in_check:
            print(f"{state.side} in Checkmate") 
        else:
            print(f"{state.side} in Stalemate")


    