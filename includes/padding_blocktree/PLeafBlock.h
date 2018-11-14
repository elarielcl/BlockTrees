//
// Created by sking32 on 5/18/18.
//

#ifndef BLOCKTREE_PLEAVEBLOCK_H
#define BLOCKTREE_PLEAVEBLOCK_H

#include "PBlock.h"

class PLeafBlock : public PBlock {
public:
    std::string data_;
    std::basic_string<int64_t> wdata_;

    PLeafBlock(PBlock*, int64_t, int64_t, int, int, std::string&, int);
    PLeafBlock(PBlock*, int64_t, int64_t, int, int, std::basic_string<int64_t>&, int);
    ~PLeafBlock();

    int add_rank_select_support(int);
    int64_t add_differential_access_support();

    int64_t better_differential_access(int);
    int64_t differential_access(int);
    int64_t differential_access_alternative(int);
    int rank(int, int);

    int test_rank(int, int, int&);

    int rank_alternative(int, int);
    int better_rank(int, int);
    int select(int, int);
    int select_alternative(int, int);
    int better_select(int, int);

    int test_fwdsearch(int, int, int&, int&);
    int test_fwdsearch(int, int, int, int&, int&);
    int test_bwdsearch(int, int, int&, int&);
    int test_bwdsearch(int, int, int, int&, int&);

    int test_min_excess(int, int, int&, int&);

    int positive_fwdsearch(int, int, int&);
    int fwdsearch(int, int, int&);
    int positive_bwdsearch(int, int, int&);
    int bwdsearch(int, int, int&);
    int min_select(int,int,int&, int&, int);
    int min_count(int, int, int&, int&);
    int min_excess(int, int, int&);
    int max_excess(int, int, int&);
    int add_fwdsearch_support();
    int add_rank_select_leaf_support();
    int add_preffix_min_count_fields();
    int add_max_fields();
    int leaf_rank(int);
    int leaf_rank_alternative(int);
    int better_leaf_rank(int);
    int leaf_select(int);
    int leaf_select_alternative(int);
    int better_leaf_select(int);

    void print();
    int access(int);
    int waccess(int);
    int access_2(int,int&);
    bool check_heuristic();
    bool check();
    int number_of_leaf_blocks();
};

#endif //BLOCKTREE_PLEAVEBLOCK_H
