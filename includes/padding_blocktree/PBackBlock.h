//
// Created by sking32 on 5/18/18.
//

#ifndef BLOCKTREE_PBACKBLOCK_H
#define BLOCKTREE_PBACKBLOCK_H

#include "PBlock.h"

class PBackBlock : public PBlock {
public:

    int prefix_min_first_block_count_;
    int prefix_min_second_block_count_;

    int prefix_first_block_min_excess_;
    int prefix_second_block_min_excess_;

    int prefix_first_block_max_excess_;
    int prefix_second_block_max_excess_;

    PBackBlock(PBlock*, int64_t, int64_t, int, int, std::string&, int, PBlock*, PBlock*, int);
    PBackBlock(PBlock*, int64_t, int64_t, int, int, std::basic_string<int64_t>&, int, PBlock*, PBlock*, int);
    ~PBackBlock();

    bool suffix_start_with_end_leaf_;
    bool prefix_start_with_end_leaf_;

    bool min_in_first_block_;
    bool min_in_both_blocks_;
    bool max_in_first_block_;
    bool max_in_both_blocks_;


    void print();
    int height();
    int access(int);
    int waccess(int);
    int access_2(int,int&);
    int number_of_selfreferences();
    bool check_heuristic();
    bool check();
    bool contains_double_pointer(bool);
    int number_of_back_blocks();
    int compute_max_hop();
    int add_rank_select_support(int);
    int64_t add_differential_access_support();

    int test_rank(int, int, int&);


    int64_t better_differential_access(int);
    int64_t differential_access(int);
    int64_t differential_access_alternative(int);
    int rank(int, int);
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
    int add_preffix_min_count_fields();
    int add_fwdsearch_support();
    int add_max_fields();
    int add_rank_select_leaf_support();
    int leaf_rank(int);
    int leaf_rank_alternative(int);
    int better_leaf_rank(int);
    int leaf_select(int);
    int leaf_select_alternative(int);
    int better_leaf_select(int);

    void put_fix_references();
};

#endif //BLOCKTREE_PBACKBLOCK_H
