//
// Created by sking32 on 5/18/18.
//

#ifndef BLOCKTREE_PCBLOCKTREE_H
#define BLOCKTREE_PCBLOCKTREE_H

#include <string>
#include <unordered_map>

#include <sdsl/bit_vectors.hpp>
#include <sdsl/vectors.hpp>
#include <sdsl/wavelet_trees.hpp>
#include "padding_blocktree/PBlockTree.h"

class PCBlockTree {
public:
    int r_; // Arity
    int leaf_length_;
    int first_level_length_;
    int number_of_levels_;
    int number_of_blocks_first_level_;
    std::string input_;

    std::vector<sdsl::bit_vector*> bt_bv_; // 1 when is Internal Block
    std::vector<sdsl::rank_support_v<1>*> bt_bv_rank_;
    std::vector<sdsl::int_vector<>*> bt_offsets_;
    sdsl::int_vector<>* leaf_string_;

    sdsl::bit_vector* leaf_bv_;
    sdsl::rank_support_v<1>* leaf_bv_rank_;

    sdsl::int_vector<>* alphabet_;
    std::unordered_map<char,int> mapping_;


    std::unordered_map<int,sdsl::int_vector<>*> bt_first_level_prefix_ranks_;
    std::unordered_map<int,sdsl::int_vector<>*> bt_first_level_cumulated_ranks_;

    std::unordered_map<int,std::vector<sdsl::int_vector<>*>> bt_prefix_ranks_;
    std::unordered_map<int,std::vector<sdsl::int_vector<>*>> bt_better_prefix_ranks_;
    std::unordered_map<int,std::vector<sdsl::int_vector<>*>> bt_cumulated_ranks_;
    std::unordered_map<int,std::vector<sdsl::int_vector<>*>> bt_ranks_;

    std::unordered_map<int,std::vector<sdsl::int_vector<>*>> bt_first_ranks_;
    std::unordered_map<int,std::vector<sdsl::int_vector<>*>> bt_second_ranks_;


    //Compressed Versions -> no significant time penalty
    std::vector<sdsl::rrr_vector<63>*> c_bt_bv_; // 1 when is Internal Block
    std::vector<sdsl::rrr_vector<63>::rank_1_type*> c_bt_bv_rank_;


    std::vector<sdsl::dac_vector_dp<sdsl::rrr_vector<>>*> c_bt_offsets_;
    sdsl::dac_vector_dp<sdsl::rrr_vector<>>* c_leaf_string_;

    sdsl::rrr_vector<63>* c_leaf_bv_;
    sdsl::rrr_vector<63>::rank_1_type* c_leaf_bv_rank_;

    std::unordered_map<int,sdsl::dac_vector_dp<sdsl::rrr_vector<>>*> c_bt_first_level_prefix_ranks_;
    std::unordered_map<int,sdsl::dac_vector_dp<sdsl::rrr_vector<>>*> c_bt_first_level_cumulated_ranks_;
    std::unordered_map<int,std::vector<sdsl::dac_vector_dp<sdsl::rrr_vector<>>*>> c_bt_prefix_ranks_;
    std::unordered_map<int,std::vector<sdsl::dac_vector_dp<sdsl::rrr_vector<>>*>> c_bt_better_prefix_ranks_;
    std::unordered_map<int,std::vector<sdsl::dac_vector_dp<sdsl::rrr_vector<>>*>> c_bt_cumulated_ranks_;
    std::unordered_map<int,std::vector<sdsl::dac_vector_dp<sdsl::rrr_vector<>>*>> c_bt_ranks_;

    std::unordered_map<int,std::vector<sdsl::dac_vector_dp<sdsl::rrr_vector<>>*>> c_bt_first_ranks_;
    std::unordered_map<int,std::vector<sdsl::dac_vector_dp<sdsl::rrr_vector<>>*>> c_bt_second_ranks_;


    //sdsl::wt_blcd<>* leaf_string_wt_;

    PCBlockTree(PBlockTree*);
    virtual ~PCBlockTree();

    int access(int);
    int rank(int, int);
    int select(int, int);


    int better_paper_rank(int, int);
    int better_paper_select(int, int);

    int c_better_paper_rank(int, int);
    int c_better_paper_select(int, int);


    int noback_rank_alternative(int, int);
    int recursive_noback_rank_alternative(int, int, int, int, int);


    int c_noback_rank_alternative(int, int);
    int c_recursive_noback_rank_alternative(int, int, int, int, int);


    int rank_alternative(int, int);
    int select_alternative(int, int);


    int optimized_rank_alternative(int, int);
    //int select_alternative(int, int);


    int noback_select_alternative(int, int);
    int c_noback_select_alternative(int, int);

    int better_rank(int, int);
    int better_select(int, int);


    int c_access(int);
    int c_rank_alternative(int, int);
    int c_select_alternative(int, int);

    virtual void print_statistics();
};


#endif //BLOCKTREE_PCBLOCKTREE_H
