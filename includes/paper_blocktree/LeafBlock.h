//
// Created by sking32 on 3/9/18.
//

#ifndef BLOCKTREE_LEAVEBLOCK_H
#define BLOCKTREE_LEAVEBLOCK_H

#include "Block.h"

class LeafBlock : public Block {
public:
    int size;
    LeafBlock(Block*, int, int, int, int, std::string&, int);
    ~LeafBlock();

    int add_rank_select_support(int);
    int rank(int, int);
    int rank_alternative(int, int);
    int better_rank(int, int);
    int select(int, int);
    int select_alternative(int, int);
    int better_select(int, int);
    void print();
    int access(int);
    int access_2(int,int&);
    bool check_heuristic();
    bool check();
    int number_of_leaf_blocks();

};

#endif //BLOCKTREE_LEAVEBLOCK_H
