//
// Created by sking32 on 3/7/18.
//

#ifndef BLOCKTREE_BLOCK_H
#define BLOCKTREE_BLOCK_H

#include <string>
#include <vector>
#include <unordered_map>

class Block {
public:
    Block* parent_;
    int start_index_;
    int end_index_; // In input string represented by the whole BlockTree
    int r_; // Arity

    std::string& source_; // Only a reference to the input is stored


    std::unordered_map<int,int> prefix_ranks_;
    std::unordered_map<int,int> ranks_;
    std::unordered_map<int,int> cumulated_ranks_;

    std::unordered_map<int,int> first_ranks_;
    std::unordered_map<int,int> second_ranks_;

    // CHECK
    Block* first_block_;
    Block* second_block_;
    int offset_;
    bool left_;
    bool right_;
    int child_number_;
    int max_leaf_length_;
    int pointing_to_me_;
    int level_index_;
    int first_occurrence_level_index_;
    int max_hop_;
    int color_;
    int fix_reference_;

    std::vector<Block*> children_;
    //CHECK

    Block(Block*, int, int, int, int, std::string&, int);
    virtual ~Block();

    //Returns the length of the string represented
    int length();
    std::string represented_string();
    virtual int add_rank_select_support(int);

    virtual int rank(int, int);
    virtual int rank_alternative(int, int);
    virtual int better_rank(int, int);


    virtual int select(int, int);
    virtual int select_alternative(int, int);
    virtual int better_select(int, int);
    virtual void print();
    virtual void put_child(int, Block*);
    virtual std::vector<Block*>& children(); //returning copies!
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

class blockcomparison
{

public:
    bool operator() (const Block*  l, const Block* r) const
    {
        return l->fix_reference_ > r->fix_reference_;
    }
};

#endif //BLOCKTREE_BLOCK_H
