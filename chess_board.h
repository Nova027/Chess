#ifndef CHESS_BOARD_H
#define CHESS_BOARD_H

#include "chess_common.h"
#include "chess_piece.h"

typedef enum castle {
    SHORT,
    LONG,
    CASTLE_MAX    // Hardcoding 2 castle-able rooks. There can be more rooks, These are just the ones you can castle with! Castle_type must be < CASTLE_MAX
} Castle_type;

class Board;

// -------------------------------------------------------------------------- Piece Info -----------------------------------------------------------------------------

class Piece {
    int u_id;                           // Unique piece id to help track and remove piece
    std::pair<int,int> pos;             // If captured, pos = {-1,-1}
    int move_count;
public:
    Board* board;
    Piece_type* type;
    Color color;

    Piece();

    Piece(Piece_type* type, Color color, std::pair<int,int> pos = {-1,-1}, Board* board = nullptr);

    int rank();

    int file();

    int& id();
};

// ----------------------------------- Wrapper class for Piece Pointer ----------------------------------

class Piece_ptr {
    Piece* piece_ptr;
    // bool processed;
    // int rank, file
public:
    Piece_ptr();

    Piece_ptr(Piece* piece_ptr);

    Piece_ptr(std::nullptr_t null);     // If more than one overload accepts a pointer type, overload for std::nullptr_t is necessary to accept a nullptr argument

    Piece_ptr(const Piece_ptr& other);
    
    Piece_ptr(Piece_ptr&& other) noexcept;

    void operator=(Piece* other);

    void operator=(const Piece_ptr& other);

    Piece* operator->();

    Piece& operator*();

    bool operator==(const Piece*& other);

    bool operator==(const Piece_ptr& other);

    bool operator!=(const Piece*& other);

    bool operator!=(const Piece_ptr& other);
};


// ------------------------------------------------------------------------ Move Info (Wrapper class) --------------------------------------------------------------------

class _Move;

class Move {
public:
    _Move* _move;
    
    Move(void* piece_types_ptr);

    Move(std::string& move, void* piece_types_ptr);

    Move(void* piece_types_ptr, int game_grid_size);

    Move(std::string& move, void* piece_types_ptr, int game_grid_size);

    ~Move();

    void reset();

    bool operator==(const Move& other);

    bool operator!=(const Move& other);

    bool operator==(const std::string& other);

    bool operator!=(const std::string& other);

    void operator=(const Move& other);

    void operator=(const std::string& other);

    char operator[](int i);

    bool is_valid();

    bool is_check();

    bool is_capture();

    Castle_type castle_type();

    Piece_type* piece_type();

    Piece_type* promo_type();

    std::pair<int,int> src();

    std::pair<int,int> dst();
};

void operator>>(std::istream& cin, Move& move);

bool operator==(const std::string& other, Move& move);

bool operator!=(const std::string& other, Move& move);


// ------------------------------------------------------------------------------ Board Info ------------------------------------------------------------------------------------

class Board {
    class Row_reference {
    public:
        std::vector<Piece_ptr>& row;

        Row_reference(std::vector<Piece_ptr>& row);

        Piece_ptr& operator[](int i);
    };

    // Actual board to denote position, a cell is NULL if empty, otherwise valid reference if some piece exists
    std::vector<std::vector<Piece_ptr>> board;

    // Set rank where pieces are placed initially. We allow castling only in this rank, regardless of where else rooks may be placed.
    int piece_ranks[Color::MAX];
    // Set rank at which pawns promote.
    int promo_ranks[Color::MAX];

    int king_pos_init;              // Initial king pos = {piece_ranks[color], king_pos_init}

    // For castle details, Just storing file info, rank would be implicit from piece-ranks. Also, castling only possible from rppre[i] to rppost[i].
    int rook_pos_precastle[CASTLE_MAX];
    int rook_pos_postcastle[CASTLE_MAX];
    std::vector<bool> can_castle[Color::MAX] = { std::vector<bool> (CASTLE_MAX, true) };

public:
    Board();

    Board(int grid_size);

    Board(int grid_size, int piece_rank_offset);

    Board(int grid_size, int piece_rank_offset, int promo_rank_offset, int pre_short, int post_short, int pre_long, int post_long);

    Row_reference operator[](int i);

    // Check if a valid board - Must have exactly 1 king per color, Must have playable pieces (not theoretical draw / not mated already)
    bool validate();

    // If there's a king or castleable rook at board[rank][file], we disable its castleability
    void remove_castle_at(int rank, int file);

    void process_inp(const Piece_ptr& piece_ptr);

    bool castle(Color& player_color, Move& move);

    bool move_pawn(Color& player_color, Move& move);

    bool Board::move_piece(Color& player_color, Move& move);

    // If the move is valid, and legal in current position, play it ; Return true. Else, return false
    bool play_if_valid(Color& player_color, Move& move);
};

#endif