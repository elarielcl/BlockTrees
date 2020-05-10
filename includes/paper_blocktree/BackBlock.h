//
// Created by sking32 on 3/9/18.
//

#ifndef BLOCKTREE_BACKBLOCK_H
#define BLOCKTREE_BACKBLOCK_H

#include "Block.h"

class BackBlock : public Block {
public:
    BackBlock(Block*, int, int, int, int, std::string&, int, Block*, Block*, int);
    ~BackBlock();

    void print();
    int height();
    int access(int);
    int access_2(int,int&);
    int number_of_selfreferences();
    bool check_heuristic();
    bool check();
    bool contains_double_pointer(bool);
    int number_of_back_blocks();
    int compute_max_hop();
    int add_rank_select_support(int);
    int rank(int, int);
    int rank_alternative(int, int);
    int better_rank(int, int);
    int select(int, int);
    int select_alternative(int, int);
    int better_select(int, int);
    void put_fix_references();

};

#endif //BLOCKTREE_BACKBLOCK_H
