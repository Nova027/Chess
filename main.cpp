#include "chess.h"
#include <iostream>

int main() {
    int remaining_moves = 3;         // Allow 1000 total moves maximum (500 each)
    
    Chess game;
    // game.start() optional unless manually added / removed pieces
    // If game.start() never was true, game never started => ongoing is false not because game over, but because not started.
    std::cout << "GAME STARTED" << std::endl;
    while (game.ongoing() && remaining_moves) {
        game.play_move();
        game.show_board();
        remaining_moves--;
        std::cout << "Moves remaining : " << remaining_moves << std::endl;
    }
    
    if (game.has_winner())
        std::cout << "The winner is " << game.winner() << std::endl;
    else if (game.is_draw())
        std::cout << "The game is a draw" << std::endl;
    else if (remaining_moves == 0)
        std::cout << "The game did not finish!" << std::endl;
    else
        std::cout << "The game did not start due to some unknown issue!" << std::endl;

    return 0;
}
