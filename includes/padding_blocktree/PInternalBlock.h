//
// Created by sking32 on 5/18/18.
//

#ifndef BLOCKTREE_PINTERNALBLOCK_H
#define BLOCKTREE_PINTERNALBLOCK_H

#include "PBlock.h"

class PInternalBlock : public PBlock {
public:

    PInternalBlock(PBlock*, int64_t, int64_t, int, int, std::string&, int);
    ~PInternalBlock();

    void print();
    std::vector<PBlock*>& children();
    void put_child(int, PBlock*);
    int number_of_nodes();
    int height();
    int number_of_leaves();
    void clean_unnecessary_expansions();
    int clean_unnecessary_expansions(int);

    void put_fix_references();
    bool is_leaf();
    int access(int);
    int access_2(int,int&);
    int number_of_selfreferences();
    bool check_heuristic();
    bool check();
    bool contains_double_pointer(bool);
    int number_of_back_blocks();
    int number_of_internal_blocks();
    int number_of_leaf_blocks();
    int compute_max_hop();
    int add_rank_select_support(int);

    int test_rank(int, int, int&);

    int rank(int, int);
    int rank_alternative(int, int);
    int better_rank(int, int);
    int select(int, int);
    int select_alternative(int, int);
    int better_select(int, int);

};

#endif //BLOCKTREE_PINTERNALBLOCK_H
