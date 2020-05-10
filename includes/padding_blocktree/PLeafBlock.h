//
// Created by sking32 on 5/18/18.
//

#ifndef BLOCKTREE_PLEAVEBLOCK_H
#define BLOCKTREE_PLEAVEBLOCK_H

#include "PBlock.h"

class PLeafBlock : public PBlock {
public:

    PLeafBlock(PBlock*, int64_t, int64_t, int, int, std::string&, int);
    ~PLeafBlock();

    int add_rank_select_support(int);
    int64_t size();
    int rank(int, int);

    int test_rank(int, int, int&);

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

#endif //BLOCKTREE_PLEAVEBLOCK_H
