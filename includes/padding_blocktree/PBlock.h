//
// Created by sking32 on 5/18/18.
//

#ifndef BLOCKTREE_PBLOCK_H
#define BLOCKTREE_PBLOCK_H

#include <string>
#include <vector>
#include <unordered_map>

class PBlock {
public:

    PBlock* parent_;
    int64_t start_index_;
    int64_t end_index_; // In input string represented by the whole BlockTree
    int r_; // Arity

    std::string& source_;

    int fix_reference_;

    std::unordered_map<int,int> prefix_ranks_;
    std::unordered_map<int,int> ranks_;
    std::unordered_map<int,int> cumulated_ranks_;

    std::unordered_map<int,int> first_ranks_;
    std::unordered_map<int,int> second_ranks_;

    // CHECK
    PBlock* first_block_;
    PBlock* second_block_;
    int offset_;
    bool left_;
    bool right_;
    int child_number_;
    int leaf_length_;
    int pointing_to_me_;
    int level_index_;
    int first_occurrence_level_index_;
    int max_hop_;
    int color_;

    std::vector<PBlock*> children_;
    //CHECK

    PBlock(PBlock*, int64_t, int64_t, int, int, std::string&, int);
    virtual ~PBlock();

    //Returns the length of the string represented
    int64_t length();
    std::string represented_string();

    virtual int add_rank_select_support(int);

    virtual int test_rank(int, int, int&);

    virtual int rank(int, int);
    virtual int rank_alternative(int, int);
    virtual int better_rank(int, int);
    virtual int select(int, int);
    virtual int select_alternative(int, int);
    virtual int better_select(int, int);

    virtual void print();
    virtual void put_child(int, PBlock*);
    virtual std::vector<PBlock*>& children(); //returning copies!
    virtual int number_of_nodes();
    virtual int height();
    virtual int number_of_leaves();
    virtual void clean_unnecessary_expansions();
    virtual int clean_unnecessary_expansions(int);

    virtual bool is_leaf();
    virtual int access(int);
    virtual int access_2(int, int&);
    virtual int number_of_selfreferences();
    virtual bool check_heuristic();
    virtual bool check();
    virtual bool contains_double_pointer(bool);
    virtual int number_of_back_blocks();
    virtual int number_of_internal_blocks();
    virtual int number_of_leaf_blocks();
    virtual int compute_max_hop();
    virtual void put_fix_references();
};

class pblockcomparison
{

public:
    bool operator() (const PBlock*  l, const PBlock* r) const
    {
        return l->fix_reference_ > r->fix_reference_;
    }
};

#endif //BLOCKTREE_PBLOCK_H
