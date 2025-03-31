
#include "chess_utils.h"
#include "chess_board.h"
#include <iostream>


// ----------------------------------------------------------------------------- Piece Info -------------------------------------------------------------------------------------

Piece::Piece() {
    type = nullptr;
    color = Color::MAX;
    pos = {-1,-1};
    board = nullptr;
    move_count = 0;
}

Piece::Piece(Piece_type* type, Color color, std::pair<int,int> pos, Board* board) {
    this->type = type;
    this->color = color;
    this->pos = pos;
    this->board = board;
    move_count = 0;
}

int Piece::rank() {
    return pos.first;
}

int Piece::file() {
    return pos.second;
}

int& Piece::id() {
    return u_id;
}

// -------------------------------- Wrapper class for Piece Pointer -----------------------------------

Piece_ptr::Piece_ptr() : piece_ptr(nullptr) {
    // std::cout << "DEFAULT CONSTRUCT" << std::endl;
}

// Need to change if-else cases, depending on what just happened. Construction, Assignment, Null construction, Null assignment
// Need to decide how I plan to handle at high level, moving a piece from one cell to another. Based on that, might need to add
// additional provisions. Also, when piece gets captured, it is removed from board pos -1,-1 not null-ed, so tracking still possible
// In that case, definitely some different/special function call is needed to process in board, as the piece is no longer on board
// Even if moved pieces need not require extra processing, captured pieces will definitely do!
// And of course new pieces added to board via construction/assignment, or pieces REMOVED (not captured), also need the same.
// Should have Destructor as well!

Piece_ptr::Piece_ptr(Piece* piece_ptr) : piece_ptr(piece_ptr) {
    // std::cout << "Piece* CONSTRUCT : ";
    if (piece_ptr && piece_ptr->board)
        piece_ptr->board->process_inp(*this);
    // else
    //     std::cout << "NOT PROCESSED" << std::endl;
}

Piece_ptr::Piece_ptr(std::nullptr_t null) : piece_ptr(null) {
    // std::cout << "NULL CONSTRUCT" << std::endl;
}

Piece_ptr::Piece_ptr(const Piece_ptr& other) : piece_ptr(other.piece_ptr) {
    // std::cout << "COPY CONSTRUCT : ";
    if (piece_ptr && piece_ptr->board)
        piece_ptr->board->process_inp(*this);
    // else
    //     std::cout << "NOT PROCESSED" << std::endl;
}

Piece_ptr::Piece_ptr(Piece_ptr&& other) noexcept : 
    piece_ptr(std::exchange(other.piece_ptr, nullptr)) {
    // std::cout << "MOVE CONSTRUCT : ";
    if (piece_ptr && piece_ptr->board)
        piece_ptr->board->process_inp(*this);
    // else
    //     std::cout << "NOT PROCESSED" << std::endl;
}

void Piece_ptr::operator=(Piece* other) {
    // std::cout << "Piece* ASSIGN : ";
    piece_ptr = other;
    if (piece_ptr && piece_ptr->board)
        piece_ptr->board->process_inp(*this);
    // else
    //     std::cout << "NOT PROCESSED" << std::endl;
}

void Piece_ptr::operator=(const Piece_ptr& other) {
    // std::cout << "COPY ASSIGN : ";       // No move assign, not sure if it would be used if present
    piece_ptr = other.piece_ptr;
    if (piece_ptr && piece_ptr->board)
        piece_ptr->board->process_inp(*this);
    // else
    //     std::cout << "NOT PROCESSED" << std::endl;
}

Piece* Piece_ptr::operator->() {
    return piece_ptr;
}

Piece& Piece_ptr::operator*() {
    return *piece_ptr;
}

bool Piece_ptr::operator==(const Piece*& other) {
    return piece_ptr == other;
}

bool Piece_ptr::operator==(const Piece_ptr& other) {
    return piece_ptr == other.piece_ptr;
}

bool Piece_ptr::operator!=(const Piece*& other) {
    return piece_ptr != other;
}

bool Piece_ptr::operator!=(const Piece_ptr& other) {
    return piece_ptr != other.piece_ptr;
}


// ---------------------------------------------------------- Move Info (Includes string parsing from SAN notation) ---------------------------------------------------------------

// _Move is hidden implementation of Move wrapper. 
// Why we need separate class: The type PType_set is only available to use in src files, and not headers
class _Move {
    PType_set& piece_types;
    int game_grid_size;
public:
    std::string move;
    
    // Below params are populated only by parsing move, src position will be filled only if specified in move string
    bool is_valid;
    // Other below params are valid, only if is_valid is true

    Castle_type castle_type;
    bool is_check;                  // If there's + or # at end
    bool is_capture;
    
    Piece_type* ptype;              // For pawn, this is set to nullptr (also default value)

    Piece_type* promo_type;         // For pawn, if promotion, what piece promoted to. ptype and promo_type cannot both be non-null!

    std::pair<int,int> src;         // {rank/row, file/col} ; Optional - {_,_} if info present, else, {grid_size,grid_size}/{_,gs}/{gs,_} ... _ is 0-(gs-1)
    std::pair<int,int> dst;         // {rank/row, file/col} ; Mandatory - {0-(gs-1),0-(gs-1)} 

    _Move(PType_set& piece_types) : piece_types(piece_types) {
        game_grid_size = 8;
        reset();
    }

    _Move(std::string& move, PType_set& piece_types) : piece_types(piece_types)  {
        game_grid_size = 8;
        reset();
        this->move = move;
        parse_move();
    }

    _Move(PType_set& piece_types, int game_grid_size) : piece_types(piece_types) {
        this->game_grid_size = game_grid_size;
        reset();
    }

    _Move(std::string& move, PType_set& piece_types, int game_grid_size) : piece_types(piece_types)  {
        this->game_grid_size = game_grid_size;
        reset();
        this->move = move;
        parse_move();
    }

    void reset() {
        move = "";
        is_valid = false;
        castle_type = CASTLE_MAX;
        is_check = false;
        is_capture = false;
        src = {game_grid_size, game_grid_size};
        dst = {game_grid_size, game_grid_size};
        ptype = nullptr;
        promo_type = nullptr;
    }

    bool operator==(const _Move& other) {
        return move == other.move;
    }

    bool operator!=(const _Move& other) {
        return move != other.move;
    }

    bool operator==(const std::string& other) {
        return move == other;
    }

    bool operator!=(const std::string& other) {
        return move != other;
    }

    void operator=(const _Move& other) {
        reset();
        move = other.move;
        parse_move();
    }

    void operator=(const std::string& other) {
        reset();
        move = other;
        parse_move();
    }

    char operator[](int i) {
        return move[i];
    }

    // TODO : Fix - Currently this has a bug, if grid size >= 10 (because it assumes rank takes one char/single digit). 
    // Fix later - Just check 2 chars (will allow max 26 for consistency)
    void parse_move() {
        std::string move = this->move;
        // Following are valid prefixes :
        // 1. O-O or O-O-O
        // 2. Source info - (Piece_type)?(src_file)?(src_rank)? All optional, Piece_type reqd if not pawn, src reqd if there's ambiguity
        //    Capture - x? Optional (present if captures)
        //    Dst info - (dst_file)(dst_rank) All mandatory
        //    Promo info - =?(Piece_type)? Optional (present if it's a pawn and it promotes)
        // Valid suffix : Either + or # or nothing.
        
        // Example Max case for pawn - cxd8=Q+ ; Max case for other piece - Nc3xd5+, either way 7.... O-O-O+ is 6.
        // Min size is 2 chars - Dst info. Enforcing both min/max
        if (move.size() < 2 || move.size() > 7) return;
        // Check for check/mate suffix
        if (move.back() == '+' || move.back() == '#') {
            move.pop_back();
            is_check = true;            // Need not validate Piece_type, as all can check (incl King, via discovery)
        }
        // Even after removing suffix, mandatory part (dst cell) must exist
        if (move.size() < 2) return;
    
        // Check if castle (short/long), early stop
        if (move == "O-O-O") castle_type = LONG;
        else if (move == "O-O") castle_type = SHORT;
        if (castle_type != CASTLE_MAX) {
            is_valid = true;
            ptype = piece_types['K'];
            return;
        }
        
        // Check if promotion at the end, first check if valid piece-type shorthand at end
        promo_type = piece_types[move.back()];
        if (promo_type != nullptr) {
            move.pop_back();
            if (move.back() != '=' || !promo_type->promotable_to)
                return;
            move.pop_back();
        }
        // Mandatory part (dst cell) must still exist
        if (move.size() < 2) return;
        
        // Check if any invalid chars in main part remaining now (P? src? x? dst), only letters & numbers allowed
        for (auto& ch : move) {
            if (!std::isalnum(ch))
                return;
        }
    
        // Look for valid dst cell at end (mandatory)
        int dst_rank = (int) move.back() - (int) '1'; move.pop_back();
        int dst_file = (int) move.back() - (int) 'a'; move.pop_back();
        if (dst_rank < 0 || dst_rank >= game_grid_size)
            return;
        if (dst_file < 0 || dst_file >= game_grid_size)
            return;
        dst = {dst_rank, dst_file};
    
        // If reached till here, and no more chars (only dst info), then valid non-capture pawn move found
        if (move.empty()) {
            is_valid = true;
            return;
        }
    
        // Checking for capture
        if (move.back() == 'x') {
            is_capture = true;
            move.pop_back();
        }
    
        // Only P? src_file? src_rank? shoule be left. But at least one char must be present (file/P), only dst pawn move already handled
        if (move.empty())
            return;
        int src_rank = game_grid_size;               // Default invalid / unspecified rank
        if (isdigit(move.back())) {
            src_rank = (int) move.back() - (int) '1';
            if (src_rank < 0 || src_rank >= game_grid_size)
                return;
            move.pop_back();
        }
        // Only P? src_file? shoule be left. But at least one char must be present (file/P)
        if (move.empty())
            return;
        int src_file = game_grid_size;               // Default invalid / unspecified file
        if (islower(move.back())) {
            src_file = (int) move.back() - (int) 'a';
            if (src_file < 0 || src_file >= game_grid_size)
                return;
            move.pop_back();
        }
        src = {src_rank, src_file};
    
        // Now should be empty if and only if pawn move
        if (move.empty()) {
            // For non-capture pawn move, src part should ALWAYS be empty in standard notation (as per chess.com article)
            // If non-capture reached here => either file/rank/both info was given in src part, so should reject it. ONLY allowing captures here
            // Even in capture, only file should be given and rank should be unspecified/empty as well in SAN (again as per article)
            if (is_capture && src_rank == game_grid_size && src_file != game_grid_size)
                is_valid = true;
            return;
        }
        // For non-pawn pieces, src disambiguation is actually allowed in SAN ONLY if ambiguity exists with dst. But I am not enforcing it.
        // I am allowing rank/file info for non-pawn pieces to be given in move string, even if no ambiguity exists.
        // Checking for ambiguity is an unnecessary & pointless overhead!
    
        // Now only 1 character should remain for piece_type (non-pawn), otherwise invalid
        if (move.size() > 1)
            return;
        // Valid if matches some piece type, and promo_type is null (as only pawns can promote, not other pieces!)
        ptype = piece_types[move[0]];
        is_valid = (ptype != nullptr) && (promo_type == nullptr);
    }
};

void operator>>(std::istream& cin, _Move& move) {
    move.reset();
    cin >> move.move;
    // If we want to allow chaining istream like std::cin >> move >> var2 >> var3 >> ..., we should return cin back
    move.parse_move();
}

bool operator==(const std::string& other, _Move& move) {
    return move.move == other;
}

// ------------------------------------------- Move : Wrapper around _Move ---------------------------------------------

Move::Move(void* piece_types_ptr) {
    _move = new _Move(*((PType_set*) piece_types_ptr));
}

Move::Move(std::string& move, void* piece_types_ptr) {
    _move = new _Move(move, *((PType_set*) piece_types_ptr));
}

Move::Move(void* piece_types_ptr, int game_grid_size) {
    _move = new _Move(*((PType_set*) piece_types_ptr), game_grid_size);
}

Move::Move(std::string& move, void* piece_types_ptr, int game_grid_size)  {
    _move = new _Move(move, *((PType_set*) piece_types_ptr), game_grid_size);
}

Move::~Move() {
    delete _move;
}

void Move::reset() {
    _move->reset();
}

bool Move::operator==(const Move& other) {
    return *_move == *(other._move);
}

bool Move::operator!=(const Move& other) {
    return *_move != *(other._move);
}

bool Move::operator==(const std::string& other) {
    return *_move == other;
}

bool Move::operator!=(const std::string& other) {
    return *_move != other;
}

void Move::operator=(const Move& other) {
    *_move = *(other._move);
}

void Move::operator=(const std::string& other) {
    *_move = other;
}

char Move::operator[](int i) {
    return (*_move)[i];
}

bool Move::is_valid() {
    return _move->is_valid;
}

bool Move::is_check() {
    return _move->is_check;
}

bool Move::is_capture() {
    return _move->is_capture;
}

Castle_type Move::castle_type() {
    return _move->castle_type;
}

Piece_type* Move::piece_type() {
    return _move->ptype;
}

Piece_type* Move::promo_type() {
    return _move->promo_type;
}

std::pair<int,int> Move::src() {
    return _move->src;
}

std::pair<int,int> Move::dst() {
    return _move->dst;
}

void operator>>(std::istream& cin, Move& move) {
    cin >> *(move._move);
}

bool operator==(const std::string& other, Move& move) {
    return other == *(move._move);
}

bool operator!=(const std::string& other, Move& move) {
    return other != *(move._move);
}


// ----------------------------------------------------------------------------- Board Info ------------------------------------------------------------------------------------------

Board::Row_reference::Row_reference(std::vector<Piece_ptr>& row) : row(row) {}

Piece_ptr& Board::Row_reference::operator[](int i) {
    static Piece_ptr null; null = nullptr;
    if (i < 0 || i >= row.size()) return null;
    return row[i];
}

Board::Board() : Board(8) {}

Board::Board(int grid_size) : Board(grid_size, 0, grid_size-1, grid_size-1, grid_size-3, 0, 3) {}

// (grid_size-1, grid_size-3, 0, 3) above (^) and below (v) --> Offset-independent castling
// Enforce specific rook ABSOLUTE positions only where castling is allowed

Board::Board(int grid_size, int piece_rank_offset) : Board(grid_size, piece_rank_offset, grid_size-1-piece_rank_offset, grid_size-1, grid_size-3, 0, 3) {}

Board::Board(int grid_size, int piece_rank_offset, int promo_rank_offset, int pre_short, int post_short, int pre_long, int post_long) {
    board = std::vector<std::vector<Piece_ptr>> (grid_size, std::vector<Piece_ptr> (grid_size, nullptr));
    piece_ranks[WHITE] = piece_rank_offset;
    piece_ranks[BLACK] = grid_size - 1 - piece_rank_offset;
    promo_ranks[WHITE] = promo_rank_offset;
    promo_ranks[BLACK] = grid_size - 1 - promo_rank_offset;
    rook_pos_precastle[SHORT] = pre_short;
    rook_pos_precastle[LONG] = pre_long;
    rook_pos_postcastle[SHORT] = post_short;   // Kingside / Short
    rook_pos_postcastle[LONG] = post_long;     // Queenside / Long
}

Board::Row_reference Board::operator[](int i) {
    static std::vector<Piece_ptr> empty; empty.clear();
    if (i < 0 || i >= board.size()) return Row_reference(empty);
    return Row_reference(board[i]);
}

bool Board::validate() {
    return true;
}

void Board::remove_castle_at(int rank, int file) {
    if ((*this)[rank][file] == nullptr)
        return;
    Piece_ptr p_ptr = board[rank][file];
    if (rank == piece_ranks[p_ptr->color]) {
        if (p_ptr->type->shorthand == 'R' && (file == rook_pos_precastle[SHORT] || file == rook_pos_precastle[LONG]))
            can_castle[p_ptr->color][(file == rook_pos_precastle[LONG])] = false;
        // (file == rook_pos_precastle[LONG]) is used to transform short_file / long_file to 0/1 respectively
        else if (p_ptr->type->shorthand == 'K')
            can_castle[p_ptr->color] = {false, false};
    }
}

void Board::process_inp(const Piece_ptr& piece_ptr) {
    // std::cout << "PROCESSED" << std::endl;
    // If piece was just added, move_count == 0, check certain things.
}

bool Board::castle(Color& player_color, Move& move) {
    if (!can_castle[player_color][move.castle_type()])
        return false;
    
    int c_rank = piece_ranks[player_color];
    int k_file_init = move.piece_type()->file_offset_default();             // Castling is considered a King-move, so move ptype is King, can find King start_pos from file_offset
    int r_file_init = rook_pos_precastle[move.castle_type()];
    
    auto king_ori_ptr = board[c_rank][k_file_init];
    auto rook_ori_ptr = board[c_rank][r_file_init];
    if (king_ori_ptr == nullptr || king_ori_ptr->type->shorthand != 'K')
        return false;
    if (rook_ori_ptr == nullptr || king_ori_ptr->type->shorthand != 'K')
        return false;
    
    // if (true);

    return true;
}

bool Board::move_pawn(Color& player_color, Move& move) {
    return true;
}

bool Board::move_piece(Color& player_color, Move& move) {
    return true;
}

// Accepts source position of moved piece/pawn ....
bool is_discovery_check() {
    return true;
}

bool is_direct_check() {
    return true;
}

bool Board::play_if_valid(Color& player_color, Move& move) {
    if (!move.is_valid())
        return false;
    
    // Handle castling completely separately, because it requires some additional condition-checks, and does not require certain checks from typical moves.
    if (move.castle_type() < CASTLE_MAX)
        return castle(player_color, move);
    
    // If there's a src position specified, check if it is within board limits
    if (move.src() != std::make_pair(-1, -1) )
        return false;

    auto [dst_rank, dst_file] = move.dst();
    auto [src_rank, src_file] = move.src();
    
    // After making the move, need to verify few things:
    // 1. Should not be in check yourself after making move
    // 2. If you claimed the move was a check in move string, must confirm it is indeed a check
    // 3. Same for capture. Also need to process the captured piece (if any) in a unified function regardless of pawn/piece capturer!

    // If dst cell is occupied by a piece of same color, cannot make the move.
    if (board[dst_rank][dst_file] != nullptr && board[dst_rank][dst_file]->color == player_color)
        return false;

    // If the move is not declared as a capture, but if the dst square is occupied, then ALSO cannot make move.
    if (!move.is_capture() && board[dst_rank][dst_file] != nullptr)
        return false;

    // If it's a pawn move
    if (move.piece_type() == nullptr) {
        if (!move_pawn(player_color, move))
            return false;
    }
    else {

    }



    return true;
}
// If current player is under check, must make a move to un-check the check. If not under check, any move is possible but it MUST NOT bring a check to yourself.
// Combining both, no need to verify if initially under check. Just need to ensure there's no check after playing the move (in both cases).

// Need to decide how to handle pawn promotion as it involves creation of a new piece. And that requires the pmap new unique id??
// No actually, what if we just modify the pawn's ptype to make it appropriate piece? YES! That way don't need any external support.