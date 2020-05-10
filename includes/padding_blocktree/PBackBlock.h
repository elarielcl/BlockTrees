//
// Created by sking32 on 5/18/18.
//

#ifndef BLOCKTREE_PBACKBLOCK_H
#define BLOCKTREE_PBACKBLOCK_H

#include "PBlock.h"

class PBackBlock : public PBlock {
public:

    PBackBlock(PBlock*, int64_t, int64_t, int, int, std::string&, int, PBlock*, PBlock*, int);
    ~PBackBlock();

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

    int test_rank(int, int, int&);


    int rank(int, int);
    int rank_alternative(int, int);
    int better_rank(int, int);
    int select(int, int);
    int select_alternative(int, int);
    int better_select(int, int);

    void put_fix_references();
};

#endif //BLOCKTREE_PBACKBLOCK_H
