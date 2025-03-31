#include "chess.h"
#include "chess_common.h"
#include "chess_piece.h"
#include "chess_board.h"
#include "chess_utils.h"
#include <iostream>

// This Class is specifically tailor-made for standard chess, that's why we have specific values/constants, and not user-defined. 
// Though still have asserts, to allow playing around with the hardcoded parameters.
// This is done to optimize piece management and move management. However, this can be taken as reference to make classes for chess variants by manipulating params,
// hardcoded piece starting positions, etc. If code duplication is a concern, may need to create a common superclass for all chess variants including std chess!
// We have the option to customize piece positions to non-standard ones using the exposed API add_piece and remove_piece. If you want to add new piece TYPES as well,
// you need to make the member piece_types as public!
class _Chess {
    const static int BOARD_SIZE;
    const static int PAWN_OFFSET , PIECE_OFFSET, START_OFFSET;
    const static std::string color_name[Color::MAX];

    Piece_type* pawn_info;
    PType_set piece_types;

    PieceID_map avl_pieces[Color::MAX];
    
    Board board;
    bool valid_game;
    
    bool ongoing_game;
    int captured_points[Color::MAX];
    Color turn;
    Move move_in;
    int win;

public:
    _Chess() : board(BOARD_SIZE, START_OFFSET + PIECE_OFFSET), move_in(&piece_types, BOARD_SIZE) {
        assert(PAWN_OFFSET != PIECE_OFFSET);
        int max_row = std::max(PAWN_OFFSET, PIECE_OFFSET);
        assert(START_OFFSET + max_row < BOARD_SIZE - 1 - START_OFFSET - max_row);

        pawn_info = new Pawn();
        piece_types.insert(new Knight());
        piece_types.insert(new Bishop());
        piece_types.insert(new Rook());
        piece_types.insert(new Queen());
        piece_types.insert(new King());
        
        reset_game();
    }

    ~_Chess() {
        delete pawn_info;
        pawn_info = nullptr;
        for (auto& ptype_ptr : piece_types)
            delete ptype_ptr;
        piece_types.clear();
    }

    // Reset to default starting position of standard chess. Board validity to be ensured after reset!
    void reset_game() {
        win = -1;
        valid_game = true;
        ongoing_game = true;
        turn = WHITE;
        std::fill_n(captured_points, Color::MAX, 0);

        for (Color c = WHITE; c < Color::MAX; c = (Color) ((int) c + 1)) {
            for (auto& [id, p] : avl_pieces[c])
                board[p.rank()][p.file()] = nullptr;
            avl_pieces[c].clear();
        }
        // std::cout << std::endl;
        for (Color c = WHITE; c < Color::MAX; c = (Color) ((int) c + 1)) {
            // Place pawns (using default for standard chess)
            int pawn_rank = (c == WHITE)? (START_OFFSET + PAWN_OFFSET) : (BOARD_SIZE - 1 - START_OFFSET - PAWN_OFFSET);
            for (int file=0; file < BOARD_SIZE; file++)
                avl_pieces[c].push_back(Piece(pawn_info, c, {pawn_rank, file}, &board));
            // Place pieces
            int piece_rank = (c == WHITE)? (START_OFFSET + PIECE_OFFSET) : (BOARD_SIZE - 1 - START_OFFSET - PIECE_OFFSET);
            for (auto& ptype : piece_types) {
                assert(ptype->count() <= 2);
                for (int i=0, start_file=0; i < ptype->count(); i++, start_file += BOARD_SIZE - 1) {
                    // 0 to 1, 1 to -1? Eqn is -2x + 1
                    int file = start_file + (1 - 2*i) * (START_OFFSET + ptype->file_offset_default());
                    assert(board[piece_rank][file] == nullptr);
                    avl_pieces[c].push_back(Piece(ptype, c, {piece_rank, file}, &board));
                }
            }
        }
        assert(board.validate());       // Asserting to ensure reset game is valid, else crash. As this does not accept user args
                                        // It is programmer's responsibility to ensure reset_game() produces valid board!
    }

    // Reset move counters for all pieces on board to 0, without moving any piece
    void reset_moves() {

    }

    // Add a piece of type with given shorthand, to board[piece_rank][piece_file], if empty. Return true if added.
    bool add_piece(char shorthand, Color c, int rank, int file) {
        if (board[rank][file] != nullptr)
            return false;                                   // Occupied cell
        if (piece_types[shorthand] == nullptr)
            return false;                                   // Invalid piece type
        
        avl_pieces[c].push_back(Piece(piece_types[shorthand], c, {rank, file}, &board));
        valid_game = false;
        return true;
    }

    // Remove piece if it exists, from board[piece_rank][piece_file]
    void remove_piece(int piece_rank, int piece_file) {
        Piece_ptr* piece_ptr_ptr = &(board[piece_rank][piece_file]);
        if (*piece_ptr_ptr == nullptr)
            return;
        
        avl_pieces[(*piece_ptr_ptr)->color].remove(piece_ptr_ptr);
        valid_game = false;
    }

    // Must manually call this after adding/removing pieces, to start game again
    bool start() {
        if (valid_game) return true;
        return valid_game = board.validate();
    }

    // This function should be called to allow next move to be taken as input from stdin/file
    void play_move() {
        if (!valid_game) return;
        do {
            std::cout << "Enter a move in std notation: ";
            std::cin >> move_in;
        } while (!board.play_if_valid(turn, move_in));
        
        turn = (Color) (((int) turn + 1) % (int) Color::MAX);       // Generic turn -- 2-player (1 - turn) is better

        // NEED TO SET WIN AFTER GAME OVER!
    }

    // This function draws the board in a file
    void show_board() {
        // 512 by 512 pixels image. Each square is 64 by 64 pixels.
    }

    bool ongoing() {
        return valid_game;
    }

    bool is_draw() {
        return win == Color::MAX;
    }

    bool has_winner() {
        return win >= 0 && win < Color::MAX;
    }

    std::string winner() {
        if (!has_winner()) return "";
        return color_name[win];
    }
};

const int _Chess::BOARD_SIZE = 8;
const int _Chess::PAWN_OFFSET = 1;                               // Assume all pawns are placed initially on the same rank (by default)
const int _Chess::PIECE_OFFSET = 0;                              // Assume all pieces are placed initially on the same rank (by default)
const int _Chess::START_OFFSET = 0;                              // Assume we start placing from 0th rank and file (and symmetrically so)
const std::string _Chess::color_name[] = {"White", "Black"};



Chess::Chess() {
    chess = new _Chess();
}

Chess::~Chess() {
    delete chess;
}

void Chess::reset_game() {
    chess->reset_game();
}

bool Chess::ongoing() {
    return chess->ongoing();
}

void Chess::play_move() {
    chess->play_move();
}

void Chess::show_board() {
    chess->show_board();
}

bool Chess::add_piece_white(char piece_shorthand, int rank, int file) {
    return chess->add_piece(piece_shorthand, WHITE, rank, file);
}

bool Chess::add_piece_black(char piece_shorthand, int rank, int file) {
    return chess->add_piece(piece_shorthand, BLACK, rank, file);
}

void Chess::remove_piece(int rank, int file) {
    chess->remove_piece(rank, file);
}

bool Chess::start() {
    return chess->start();
}

bool Chess::has_winner() {
    return chess->has_winner();
}

bool Chess::is_draw() {
    return chess->is_draw();
}

std::string Chess::winner() {
    return chess->winner();
}