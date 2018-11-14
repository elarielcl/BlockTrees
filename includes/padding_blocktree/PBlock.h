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
    std::string aux;
    std::basic_string<int64_t> waux;


    PBlock* parent_;
    int64_t start_index_;
    int64_t end_index_; // In input string represented by the whole BlockTree
    int r_; // Arity

    std::string& source_;
    std::basic_string<int64_t>& wsource_;

    int fix_reference_;



    int64_t first_sum_;
    int64_t second_sum_;

    int64_t sum_;
    int64_t prefix_sum_;
    int64_t cumulated_sum_;

    int leaf_rank_;
    int first_leaf_rank_;
    int second_leaf_rank_;
    int prefix_leaf_rank_;
    int cumulated_leaf_rank_;
    bool starts_with_end_leaf_;

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
    int min_prefix_excess_;
    int max_prefix_excess_;
    int prefix_min_count_;

    std::vector<PBlock*> children_;
    //CHECK

    PBlock(PBlock*, int64_t, int64_t, int, int, std::string&, int);
    PBlock(PBlock*, int64_t, int64_t, int, int, std::basic_string<int64_t>&, int);
    virtual ~PBlock();

    //Returns the length of the string represented
    int64_t length();
    int excess();
    int min_excess();
    int max_excess();
    int min_count();
    int linear_min_excess(int, int);
    int linear_max_excess(int, int);
    int linear_min_count(int, int);
    int linear_min_select(int, int, int);
    std::string represented_string();
    std::basic_string<int64_t> represented_wstring();

    virtual int add_rank_select_support(int);
    virtual int add_rank_select_leaf_support();
    virtual int add_fwdsearch_support();
    virtual int add_max_fields();
    virtual int add_preffix_min_count_fields();

    virtual int64_t add_differential_access_support();

    virtual int test_fwdsearch(int, int, int&, int&);
    virtual int test_fwdsearch(int, int, int, int&, int&);
    virtual int test_bwdsearch(int, int, int&, int&);
    virtual int test_bwdsearch(int, int, int, int&, int&);

    virtual int test_min_excess(int, int, int&, int&);

    virtual int min_select(int,int,int&, int&, int);
    virtual int min_count(int, int, int&, int&);
    virtual int min_excess(int, int, int&);
    virtual int max_excess(int, int, int&);
    virtual int positive_bwdsearch(int, int, int&);
    virtual int bwdsearch(int, int, int&);
    virtual int positive_fwdsearch(int, int, int&);
    virtual int fwdsearch(int, int, int&);

    virtual int test_rank(int, int, int&);

    virtual int rank(int, int);
    virtual int rank_alternative(int, int);
    virtual int better_rank(int, int);
    virtual int select(int, int);
    virtual int select_alternative(int, int);
    virtual int better_select(int, int);


    virtual int leaf_rank(int);
    virtual int leaf_rank_alternative(int);
    virtual int better_leaf_rank(int);
    virtual int leaf_select(int);
    virtual int leaf_select_alternative(int);
    virtual int better_leaf_select(int);

    virtual void print();
    virtual void put_child(int, PBlock*);
    virtual std::vector<PBlock*>& children(); //returning copies!
    virtual std::vector<PBlock*>& wchildren(); //returning copies!
    virtual int number_of_nodes();
    virtual int height();
    virtual int number_of_leaves();
    virtual void clean_unnecessary_expansions();
    virtual int clean_unnecessary_expansions(int);

    virtual void wclean_unnecessary_expansions();
    virtual int wclean_unnecessary_expansions(int);

    virtual bool is_leaf();
    virtual int access(int);
    virtual int waccess(int);
    virtual int64_t better_differential_access(int);
    virtual int64_t differential_access(int);
    virtual int64_t differential_access_alternative(int);
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
