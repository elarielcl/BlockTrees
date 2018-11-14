//
// Created by sking32 on 3/9/18.
//

#ifndef BLOCKTREE_INTERNALBLOCK_H
#define BLOCKTREE_INTERNALBLOCK_H

#include "Block.h"

class InternalBlock : public Block {
public:



    InternalBlock(Block*, int, int, int, int, std::string&, int, int);
    ~InternalBlock();

    void print();
    std::vector<Block*>& children();
    void put_child(int, Block*);
    int number_of_nodes();
    int height();
    int number_of_leaves();
    void clean_unnecessary_expansions();
    int clean_unnecessary_expansions(int);
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
    int add_rank_select_leaf_support();
    int add_rank_select_support(int);
    int rank(int, int);
    int rank_alternative(int, int);
    int better_rank(int, int);
    int select(int, int);
    int select_alternative(int, int);
    int better_select(int, int);

    int leaf_rank(int);
    int leaf_rank_alternative(int);
    int better_leaf_rank(int);
    int leaf_select(int);
    int leaf_select_alternative(int);
    int better_leaf_select(int);
    void put_fix_references();
};

#endif //BLOCKTREE_INTERNALBLOCK_H
