// Wrappers and utilities based on stuff available in the other headers; To be included in src files only!
#include "chess_piece.h"
#include "chess_board.h"
#include "chess_common.h"

// To use custom hash and comparison functions internally for unordered_set usage, overloading definitions of 
// template <> struct hash<Piece_type*> and template <> struct equal_to<Piece_type*> 
// (Benefit of this is that directly doing unordered_set<Piece_type*> set; would work without providing hash & equal_to structs)
// HOWEVER, Overloading in std namespace is violation of ODR, preferably do it in your own namespace... 
// Doing it manually in separate ns below
namespace chess_ns {
    struct ptype_hash {
        using is_transparent = void;                    // To enable heterogenous lookup within set (C++20 onward)

        size_t operator()(const Piece_type* type) const {
            if (type == nullptr) return 0;
            return std::hash<char>()(type->shorthand);  // Hash based on 'shorthand'
        }

        size_t operator()(char shorthand) const {
            return std::hash<char>()(shorthand);        // Allow hashing a 'char' directly
        }
    };
    
    struct ptype_equal {
        using is_transparent = void;                    // To enable heterogenous lookup within set (C++20 onward)

        bool operator()(const Piece_type* lhs, const Piece_type* rhs) const {
            if (lhs == rhs)
                return true;
            if (lhs == nullptr || rhs == nullptr)
                return false;
            return lhs->shorthand == rhs->shorthand;  // Compare based on 'shorthand'
        }

        bool operator()(const Piece_type* lhs, char rhs_shorthand) const {
            return lhs && lhs->shorthand == rhs_shorthand;       // Allow comparing Piece_type* with char
        }
    };
}

class PType_set {
    std::unordered_set<Piece_type*, chess_ns::ptype_hash, chess_ns::ptype_equal> piece_types;
public:
    void insert(Piece_type* ptype) {
        // Don't reinsert if shorthand is same, considered assert-crashing here, but ehh.
        if (piece_types.find(ptype) == piece_types.end())
            piece_types.insert(ptype);
    }

    Piece_type* operator[](char shorthand) {
        auto it = piece_types.find(shorthand);
        if (it == piece_types.end())
            return nullptr;
        return *it;
    }

    auto begin() {
        return piece_types.begin();
    }

    auto end() {
        return piece_types.end();
    }

    void clear() {
        piece_types.clear();
    }
};



// DS to have push_back() and [] operator. Internally stores into and accesses a hashmap. However, push_back(val) is to store {key, val} in hashmap,
// where key is the smallest available key (which is basically called piece_id here) So, appropriate name is Piece_map?
class PieceID_map {
    // Have to assign unique ids, not practical to store all available ids, but assume it is practical to store all assigned ids somehow
    // Say initially 1 to n is assigned, say we track in some map.
    // Say we remove k1, k2, k3  where they are numbers between 1 to n .We want to able to assign(min(k1,k2,k3)) then next min, the nmax, then assign n+1!

    // Well we can store the "freed" ids in a heap, and assign from heap by default if empty. If heap is empty, we assign max assigned id so far (which is tracked) + 1.
    // It is easy to see that the map size will never exceed reasonable amount as it only stores currently assigned ids. 
    // Also, heap size < the largest id ever present in map. So, no issue!
    std::unordered_map<int, Piece> pmap;
    std::priority_queue<int, std::vector<int>, std::greater<int>> min_avl_id;

    int max_assigned_so_far;            // Not current max id, but max id ever assigned, as current max might be lower, as max ids can also be moved to heap

public:
    PieceID_map() {
        clear();
    }

    const Piece_ptr& operator[](int uid) {
        if (pmap.find(uid) == pmap.end())
            return nullptr;
        return &(pmap[uid]);
    }

    int operator[](Piece_ptr p) {
        if (p == nullptr) return -1;
        return p->id();
    }

    auto begin() {
        return pmap.begin();
    }

    auto end() {
        return pmap.end();
    }

    void clear() {
        pmap.clear();
        min_avl_id = std::priority_queue<int, std::vector<int>, std::greater<int>>();
        max_assigned_so_far = -1;
    }

    // Accept a Piece rvalue reference to store into piece map after assigning min avl id to it, place Piece_ptr on board (if exists)
    void push_back(Piece&& in_p) {
        int id;
        if (min_avl_id.empty()) {
            id = max_assigned_so_far;
            max_assigned_so_far++;
        }
        else {
            id = min_avl_id.top();
            min_avl_id.pop();
        }

        in_p.id() = id;
        Piece_ptr ptr;
        Piece_ptr& ptr_ref = ptr;
        if (in_p.board)
            ptr_ref = in_p.board->operator[](in_p.rank())[in_p.file()];     // Reference now points to board[i][j] (static null, if invalid i,j)

        pmap[id] = std::move(in_p);
        ptr_ref = &pmap[id];                // Reassign reference variable (hence modifying the original) to now contain value Piece_ptr(Piece*)
    }

    // Remove the Piece from pmap, and also set the pointer at *piece_ptr_ptr as null
    void remove(Piece_ptr* piece_ptr_ptr) {
        if (piece_ptr_ptr == nullptr || *piece_ptr_ptr == nullptr)
            return;
        int id = (*piece_ptr_ptr)->id();
        *piece_ptr_ptr = nullptr;
        pmap.erase(id);
        min_avl_id.push(id);
    }
};