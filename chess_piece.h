#ifndef CHESS_PIECE_H
#define CHESS_PIECE_H

#include "chess_common.h"

// Abstract class to implement pieces - Future feature is to allow modifying certain parameters for each piece-type to customize game from Chess variant class
class Piece_type {
protected:
    // Following 2 are only to enable the default offset variable, which is to allow setting pieces (not pawns) in default starting position(s) during board reset
    // Since STANDARD chess only has max 2 non-pawn pieces per piece type, count should be max 2. Expect assert crash otherwise when setting pieces.
    int cnt;          // In STANDARD chess, how many of this piece are present.
    int offset;       // If count is 2, files are a + offset and last(default h) - offset. If count is 1, just singular position a + offset is considered.
public:
    int points;
    bool promotable_to;
    std::string name;
    char shorthand;

    virtual bool is_legal_move(std::pair<int,int> src, std::pair<int,int> dst) = 0;

    virtual ~Piece_type() {}

    int count() {
        return cnt;
    }

    int file_offset_default() {
        return offset;
    }
};

// Pawn is a special type of Piece_type, which occurs many times and assumed to have same moveset in all chess variants.
// It is also considered the only piece type which is implicitly capable of promotion and en passant.
class Pawn : public Piece_type {
public:
    Pawn() {
        points = 1;
        promotable_to = false;
        name = "Pawn";
        shorthand = (char) NULL;       // Pawn does not have a valid shorthand ('\0' cannot be given as user-input via cin)
    }

    // Need to have this function as it's implementing abstract class, but may be unused for optimization (for current usecase, as pawns have consistent moveset)
    // In other words, pawn move logic will be hardcoded to the board itself, tracking all pawns at once, instead each at a time!
    bool is_legal_move(std::pair<int,int> src, std::pair<int,int> dst) override;
};

class Knight : public Piece_type {
public:
    Knight() {
        points = 3;
        promotable_to = true;
        name = "Knight";
        shorthand = 'N';

        cnt = 2;
        offset = 1;
    }

    bool is_legal_move(std::pair<int,int> src, std::pair<int,int> dst) override;
};
    
class Bishop : public Piece_type {
public:
    Bishop() {
        points = 3;
        promotable_to = true;
        name = "Bishop";
        shorthand = 'B';

        cnt = 2;
        offset = 2;
    }

    bool is_legal_move(std::pair<int,int> src, std::pair<int,int> dst) override;
};
    
class Rook : public Piece_type {
public:
    Rook() {
        points = 5;
        promotable_to = true;
        name = "Rook";
        shorthand = 'R';

        cnt = 2;
        offset = 0;
    }

    bool is_legal_move(std::pair<int,int> src, std::pair<int,int> dst) override;
};

class Queen : public Piece_type {
public:
    Queen() {
        points = 9;
        promotable_to = true;
        name = "Queen";
        shorthand = 'Q';

        cnt = 1;
        offset = 3;
    }

    bool is_legal_move(std::pair<int,int> src, std::pair<int,int> dst) override;
};

class King : public Piece_type {
public:
    King() {
        points = 0;
        promotable_to = false;
        name = "King";
        shorthand = 'K';

        cnt = 1;
        offset = 4;
    }

    bool is_legal_move(std::pair<int,int> src, std::pair<int,int> dst) override;
};

#endif