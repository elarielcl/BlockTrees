//
// Created by sking32 on 5/18/18.
//

#ifndef BLOCKTREE_PINTERNALBLOCK_H
#define BLOCKTREE_PINTERNALBLOCK_H

#include "PBlock.h"

class PInternalBlock : public PBlock {
public:

    PInternalBlock(PBlock*, int64_t, int64_t, int, int, std::string&, int);
    PInternalBlock(PBlock*, int64_t, int64_t, int, int, std::basic_string<int64_t>&, int);
    ~PInternalBlock();

    void print();
    std::vector<PBlock*>& children();
    std::vector<PBlock*>& wchildren();
    void put_child(int, PBlock*);
    int number_of_nodes();
    int height();
    int number_of_leaves();
    void clean_unnecessary_expansions();
    int clean_unnecessary_expansions(int);

    void wclean_unnecessary_expansions();
    int wclean_unnecessary_expansions(int);
    void put_fix_references();
    bool is_leaf();
    int access(int);
    int waccess(int);
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
    int add_fwdsearch_support();
    int add_preffix_min_count_fields();
    int add_max_fields();
    int add_rank_select_leaf_support();
    int leaf_rank(int);
    int leaf_rank_alternative(int);
    int better_leaf_rank(int);
    int leaf_select(int);
    int leaf_select_alternative(int);
    int better_leaf_select(int);
};

#endif //BLOCKTREE_PINTERNALBLOCK_H
