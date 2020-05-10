//
// Created by sking32 on 5/18/18.
//

#ifndef BLOCKTREE_PBLOCKTREE_H
#define BLOCKTREE_PBLOCKTREE_H

#include <string>
#include <unordered_map>

#include "blocktree.utils/RabinKarp.h"
#include "blocktree.utils/NonConsecutiveRabinKarp.h"
#include "blocktree.utils/BackwardRabinKarp.h"
#include "blocktree.utils/NonConsecutiveHashString.h"
#include "blocktree.utils/HashString.h"
#include "PBackBlock.h"
#include "PBlock.h"

class PBlockTree {
    void block_scan(std::vector<PBlock*>&, int, std::unordered_map<HashString, std::vector<PBlock*>>&);
public:
    int r_; // Arity
    int leaf_length_;
    std::string input_; // Input sequence of the Tree
    PBlock* root_block_;

    PBlockTree(std::string&, int, int);
    ~PBlockTree();

    // Returns a vector of levels of nodes of the tree where
    // each level is represented by a vector of its nodes (left-to-right).
    //
    // A simple levelwise (left-to-right) traversal of the tree would be:
    //     for (std::vector<Block*> level : bt->levelwise_iterator()) {
    //         for (Block* b : level) {
    //             ...
    std::vector<std::vector<PBlock*>> levelwise_iterator();
    int number_of_nodes();
    int height();
    int number_of_leaves();
    int number_of_internal_blocks();
    int number_of_back_blocks();
    int number_of_leaf_blocks();
    void clean_unnecessary_expansions();
    void clean_unnecessary_expansions(int);


    int access(int);
    int access_2(int,int&);
    int number_of_selfreferences();
    bool check_heuristic();
    bool check();
    bool contains_double_pointer();
    int get_max_hop();
    void print_statistics();
    void print_statistics_2();
    void add_rank_select_support(int);
    int rank(int, int);
    int rank_alternative(int, int);
    int better_rank(int, int);
    int select(int, int);
    int select_alternative(int, int);
    int better_select(int, int);


    void process_back_pointers_conservative_optimized_heuristic();
    void process_back_pointers_conservative_heuristic();
    void process_back_pointers_conservative_heuristic(int);
    void process_back_pointers_reverse_conservative_heuristic(int);
    void process_back_pointers_liberal_heuristic();

    void process_back_pointers_heuristic_concatenate();
    void process_back_pointers_heuristic();
    void process_back_pointers();


    void process_real_back_front_pointers();
    void process_level_real_back_front(std::vector<PBlock*>&);
    void forward_window_block_scan_real_back_and_front(std::vector<PBlock*>& level, int window_size, int N, std::unordered_map<HashString, std::vector<PBlock*>>& hashtable);
    void process_back_front_pointers();
    void process_level_back_front(std::vector<PBlock*>&);


    void process_real_other_back_front_pointers();
    void process_level_real_other_back_front(std::vector<PBlock*>&);
    void forward_window_block_scan_real_other_back_and_front(std::vector<PBlock*>& level, int window_size, int N, std::unordered_map<HashString, std::vector<PBlock*>>& hashtable);
    void process_other_back_front_pointers();
    void process_level_other_back_front(std::vector<PBlock*>&);

    void process_back_pointers_real_conservative_heuristic();
    void process_level_real_conservative_heuristic(std::vector<PBlock*>&);
    void forward_window_block_scan_real_conservative_heuristic(std::vector<PBlock*>& level, int window_size, int N, std::unordered_map<HashString, std::vector<PBlock*>>& hashtable);

    void process_back_pointers_real_other_conservative_heuristic();
    void process_level_real_other_conservative_heuristic(std::vector<PBlock*>&);
    void forward_window_block_scan_real_other_conservative_heuristic(std::vector<PBlock*>& level, int window_size, int N, std::unordered_map<HashString, std::vector<PBlock*>>& hashtable);
    void process_back_pointers_other_conservative_heuristic();
    void process_level_other_conservative_heuristic(std::vector<PBlock*>&);


    void process_back_pointers_real_conservative_optimized_heuristic();
    void process_level_real_conservative_optimized_heuristic(std::vector<PBlock*>&);
    void forward_window_block_scan_real_conservative_optimized_heuristic(std::vector<PBlock*>& level, int window_size, int N, std::unordered_map<HashString, std::vector<PBlock*>>& hashtable);

    void process_back_pointers_real_conservative_heuristic(int);
    void process_level_real_conservative_heuristic(std::vector<PBlock*>&, int);
    void forward_window_block_scan_real_conservative_heuristic(std::vector<PBlock*>& level, int window_size, int N, std::unordered_map<HashString, std::vector<PBlock*>>& hashtable,int);

    void process_level_conservative_optimized_heuristic(std::vector<PBlock*>&);
    void process_level_conservative_heuristic(std::vector<PBlock*>&);
    void process_level_conservative_heuristic(std::vector<PBlock*>&, int);
    void process_level_reverse_conservative_heuristic(std::vector<PBlock*>&, int);
    void process_level_liberal_heuristic(std::vector<PBlock*>&);

    void process_level_heuristic_concatenate(std::vector<PBlock*>&);
    void process_level_heuristic(std::vector<PBlock*>&);
    void process_level(std::vector<PBlock*>&);


    void forward_window_block_scan_front(std::vector<PBlock*>& level, int window_size, int N, std::unordered_map<HashString, std::vector<PBlock*>>& hashtable);
    void forward_window_block_scan_heuristic_concatenate(std::vector<PBlock*>&, int, int, std::unordered_map<NonConsecutiveHashString, std::vector<PBlock*>>&);
    void forward_window_block_scan(std::vector<PBlock*>& level, int window_size, int N, std::unordered_map<HashString, std::vector<PBlock*>>& hashtable);
    void forward_pair_window_block_scan(std::vector<PBlock*>& level, int pair_window_size, int N, std::unordered_map<HashString, std::vector<std::pair<PBlock*, PBlock*>>>& pair_hashtable);

    std::vector<PBlock*> next_level(std::vector<PBlock*>&);
    void clean_self_references(std::vector<PBlock*>&);
    void update_pointing_to_me(std::vector<PBlock*>&);
};


//deprecated: fix problems fixed in the forward construction
void backward_window_block_scan(std::vector<PBlock*>& level, int window_size, PBlockTree* bt, int N, std::unordered_map<HashString, std::vector<PBlock*>>& hashtable);

void backward_pair_window_block_scan(std::vector<PBlock*>& level, int pair_window_size, PBlockTree* bt, int N, std::unordered_map<HashString, std::vector<std::pair<PBlock*, PBlock*>>>& pair_hashtable);

#endif //BLOCKTREE_PBLOCKTREE_H
