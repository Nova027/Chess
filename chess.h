#include <string>

class _Chess;           // Hidden implementation

class Chess {
    _Chess* chess;
public:
    Chess();

    ~Chess();

    void reset_game();

    bool ongoing();

    void play_move();

    void show_board();

    bool add_piece_white(char piece_shorthand, int rank, int file);

    bool add_piece_black(char piece_shorthand, int rank, int file);

    void remove_piece(int rank, int file);

    bool start();

    bool has_winner();

    bool is_draw();

    std::string winner();
};