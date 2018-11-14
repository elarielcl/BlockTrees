//
// Created by sking32 on 5/31/18.
//
#include <bits/stdc++.h>
#include <padding_blocktree/PLeafBlock.h>
#include <padding_blocktree/PLazyInternalBlock.h>
#include <padding_blocktree/PInternalBlock.h>
#include <unordered_set>
#include <fstream>
#include <compressed/PCBlockTree.h>
#include <compressed/BPSPCBlockTree.h>
#include "gtest/gtest.h"

#include "padding_blocktree/PBlockTree.h"

using ::testing::Combine;
using ::testing::Values;

typedef PBlockTree* CreateBlockTreeFunc(int, int, std::string);

PBlockTree* block_tree(int r, int max_leaf_length, std::string input) {

    PBlockTree* block_tree_ = new PBlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers();
    block_tree_->clean_unnecessary_expansions();
    return block_tree_;
}

PBlockTree* block_tree_without_cleanning(int r, int max_leaf_length, std::string input) {
    PBlockTree* block_tree_ = new PBlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers();
    return block_tree_;
}

PBlockTree* heuristic_concatenate_block_tree(int r, int max_leaf_length, std::string input) {
    PBlockTree* block_tree_ = new PBlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers_heuristic_concatenate();
    return block_tree_;
}


PBlockTree* heuristic_block_tree(int r, int max_leaf_length, std::string input) {
    PBlockTree* block_tree_ = new PBlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers_heuristic();
    return block_tree_;
}



PBlockTree* liberal_heuristic_block_tree(int r, int max_leaf_length, std::string input) {
    PBlockTree* block_tree_ = new PBlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers_liberal_heuristic();
    return block_tree_;
}


PBlockTree* conservative_heuristic_block_tree(int r, int max_leaf_length, std::string input) {
    PBlockTree* block_tree_ = new PBlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers_conservative_heuristic();
    return block_tree_;
}


PBlockTree* conservative_heuristic_3_block_tree(int r, int max_leaf_length, std::string input) {
    PBlockTree* block_tree_ = new PBlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers_conservative_heuristic(3);
    return block_tree_;
}


PBlockTree* conservative_optimized_heuristic_block_tree(int r, int max_leaf_length, std::string input) {
    PBlockTree* block_tree_ = new PBlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers_conservative_optimized_heuristic();
    return block_tree_;
}

class BPSPCBlockTreeFixture : public ::testing::TestWithParam<::testing::tuple<int, int, std::string, CreateBlockTreeFunc*>> {
protected:
    virtual void TearDown() {
        delete block_tree_;
        delete bps_;
    }

    virtual void SetUp() {
        CreateBlockTreeFunc* create_blocktree = ::testing::get<3>(GetParam());
        r_ = ::testing::get<0>(GetParam());
        max_leaf_length_ = ::testing::get<1>(GetParam());

        std::ifstream t(::testing::get<2>(GetParam()));
        std::stringstream buffer;
        buffer << t.rdbuf();
        input_= buffer.str();
        block_tree_ = (*create_blocktree)(r_ , max_leaf_length_, input_);
        std::unordered_set<int> characters;
        for (char c: input_)
            characters.insert(c);
        for (int c: characters) {
            block_tree_->add_rank_select_support(c);
        }
        block_tree_->add_rank_select_leaf_support();
        block_tree_->add_fwdsearch_support();
        block_tree_->add_max_fields();
        block_tree_->add_preffix_min_count_fields();

        bps_ = new BPSPCBlockTree(block_tree_);
    }

public:
    PBlockTree* block_tree_;

    BPSPCBlockTree* bps_;


    std::string input_;
    int r_;
    int max_leaf_length_;

    BPSPCBlockTreeFixture() : ::testing::TestWithParam<::testing::tuple<int, int, std::string, CreateBlockTreeFunc*>>() {
    }

    virtual ~BPSPCBlockTreeFixture() {
    }
};

INSTANTIATE_TEST_CASE_P(BPSPCBlockTreeTest,
                        BPSPCBlockTreeFixture,
                        Combine(Values(2),
                                Values(4),
                                Values("../../tests/data/dna.par"/*, "../../tests/data/einstein"*/),
                                Values(&block_tree, &block_tree_without_cleanning, &heuristic_concatenate_block_tree, &heuristic_block_tree,
                                       &liberal_heuristic_block_tree, &conservative_heuristic_block_tree, &conservative_heuristic_3_block_tree, &conservative_optimized_heuristic_block_tree)));


// This test checks the excess method for every
// position in the input
TEST_P(BPSPCBlockTreeFixture, excess_check) {
    int e = 0;
    for (int i = 0; i < input_.length(); ++i) {
        e += (input_[i] == '(') ? 1 : -1;
        EXPECT_EQ(bps_->excess(i), e);
    }
}


// This test checks the rank_10 method for every
// position in the input
TEST_P(BPSPCBlockTreeFixture, rank_10_check) {
    int r = 0;
    bool one_seen = false;
    for (int i = 0; i < input_.length(); ++i) {
        if (input_[i] == input_[0]) {
            one_seen = true;
        } else {
            if (one_seen) {
                ++r;
            }
            one_seen = false;
        }
        EXPECT_EQ(bps_->rank_10(i), r);
    }
}


// This test checks the rank_10_alternative method for every
// position in the input
TEST_P(BPSPCBlockTreeFixture, rank_10_alternative_check) {
    int r = 0;
    bool one_seen = false;
    for (int i = 0; i < input_.length(); ++i) {
        if (input_[i] == input_[0]) {
            one_seen = true;
        } else {
            if (one_seen) {
                ++r;
            }
            one_seen = false;
        }
        EXPECT_EQ(bps_->rank_10_alternative(i), r);
    }
}

// This test checks the better_rank_10 method for every
// position in the input
TEST_P(BPSPCBlockTreeFixture, better_rank_10_check) {
    int r = 0;
    bool one_seen = false;
    for (int i = 0; i < input_.length(); ++i) {
        if (input_[i] == input_[0]) {
            one_seen = true;
        } else {
            if (one_seen) {
                ++r;
            }
            one_seen = false;
        }
        EXPECT_EQ(bps_->better_rank_10(i), r);
    }
}


// This test checks the select_10 method for every
// position in the input
TEST_P(BPSPCBlockTreeFixture, select_10_check) {
    int r = 0;
    bool one_seen = false;
    for (int i = 0; i < input_.length(); ++i) {
        if (input_[i] == input_[0]) {
            one_seen = true;
        } else {
            if (one_seen) {
                ++r;
                EXPECT_EQ(bps_->select_10(r), i-1);
            }
            one_seen = false;
        }

    }
}


// This test checks the select_10_alternative method for every
// position in the input
TEST_P(BPSPCBlockTreeFixture, select_10_alternative_check) {
    int r = 0;
    bool one_seen = false;
    for (int i = 0; i < input_.length(); ++i) {
        if (input_[i] == input_[0]) {
            one_seen = true;
        } else {
            if (one_seen) {
                ++r;
                EXPECT_EQ(bps_->select_10_alternative(r), i-1);
            }
            one_seen = false;
        }
    }
}

// This test checks the better_select_10 method for every
// position in the input
TEST_P(BPSPCBlockTreeFixture, better_select_10_check) {
    int r = 0;
    bool one_seen = false;
    for (int i = 0; i < input_.length(); ++i) {
        if (input_[i] == input_[0]) {
            one_seen = true;
        } else {
            if (one_seen) {
                ++r;
                EXPECT_EQ(bps_->better_select_10(r), i-1);
            }
            one_seen = false;
        }
    }
}

// This test checks the fwdsearch method for every character
// in the input and d = {-1, -2} works correctly
TEST_P(BPSPCBlockTreeFixture, fwdsearch_check) {
    std::unordered_set<int> ds = {-1, -2};
    for (int d : ds) {
        for (int i = 0; i < input_.length(); ++i) {
            int search = bps_->fwdsearch(i, d);
            int excess = 0;
            int j = i;
            while (true) {
                ++j;
                if (j == input_.length()) break;
                excess += (input_[0] == input_[j]) ? 1 : -1;
                if (excess == d) break;
            }
            EXPECT_EQ(search, j);
        }
    }
}

// This test checks the my_fwdsearch method for every character
// in the input and d = {-1, -2} works correctly
TEST_P(BPSPCBlockTreeFixture, my_fwdsearch_check) {
    std::unordered_set<int> ds = {-1, -2};
    for (int d : ds) {
        for (int i = 0; i < input_.length(); ++i) {
            int search = bps_->my_fwdsearch(i, d);
            int excess = 0;
            int j = i;
            while (true) {
                ++j;
                if (j == input_.length()) break;
                excess += (input_[0] == input_[j]) ? 1 : -1;
                if (excess == d) break;
            }
            EXPECT_EQ(search, j);
        }
    }
}


// This test checks the bwdsearch method for every character
// in the input and d = {-1, -2} works correctly
TEST_P(BPSPCBlockTreeFixture, bwdsearch_check) {
    std::unordered_set<int> ds = {-1, -2};
    for (int d : ds) {
        for (int i = 0; i < input_.length(); ++i) {
            int search = bps_->bwdsearch(i, d);
            int excess = 0;
            int j = i;
            while (true) {
                if (j == 0) break;
                excess += (input_[0] == input_[j]) ? 1 : -1;
                if (excess == -d) break;
                --j;
            }
            EXPECT_EQ(search, j-1);
        }
    }
}


// This test checks the my_bwdsearch method for every character
// in the input and d = {-1, -2} works correctly
TEST_P(BPSPCBlockTreeFixture, my_bwdsearch_check) {
    std::unordered_set<int> ds = {-1, -2};
    for (int d : ds) {
        for (int i = 0; i < input_.length(); ++i) {
            int search = bps_->my_bwdsearch(i, d);
            int excess = 0;
            int j = i;
            while (true) {
                if (j == 0) break;
                excess += (input_[0] == input_[j]) ? 1 : -1;
                if (excess == -d) break;
                --j;
            }
            EXPECT_EQ(search, j-1);
        }
    }
}

// This test checks the my_bwdsearch_cback method for every character
// in the input and d = {-1, -2} works correctly
TEST_P(BPSPCBlockTreeFixture, my_bwdsearch_cback_check) {
    std::unordered_set<int> ds = {-1, -2};
    for (int d : ds) {
        for (int i = 0; i < input_.length(); ++i) {
            int search = bps_->my_bwdsearch_cback(i, d);
            int excess = 0;
            int j = i;
            while (true) {
                if (j == 0) break;
                excess += (input_[0] == input_[j]) ? 1 : -1;
                if (excess == -d) break;
                --j;
            }
            EXPECT_EQ(search, j-1);
        }
    }
}


// This test checks the positive_fwdsearch method for every character
// in the input and d = {1, 2} works correctly
TEST_P(BPSPCBlockTreeFixture, positive_fwdsearch_check) {
    std::unordered_set<int> ds = {1, 2};
    for (int d : ds) {
        for (int i = 0; i < input_.length(); ++i) {
            int search = bps_->positive_fwdsearch(i, d);
            int excess = 0;
            int j = i;
            while (true) {
                ++j;
                if (j == input_.length()) break;
                excess += (input_[0] == input_[j]) ? 1 : -1;
                if (excess == d) break;
            }
            EXPECT_EQ(search, j);
        }
    }
}

// This test checks the my_positive_fwdsearch method for every character
// in the input and d = {1, 2} works correctly
TEST_P(BPSPCBlockTreeFixture, my_positive_fwdsearch_check) {
    std::unordered_set<int> ds = {1, 2};
    for (int d : ds) {
        for (int i = 0; i < input_.length(); ++i) {
            int search = bps_->my_positive_fwdsearch(i, d);
            int excess = 0;
            int j = i;
            while (true) {
                ++j;
                if (j == input_.length()) break;
                excess += (input_[0] == input_[j]) ? 1 : -1;
                if (excess == d) break;
            }
            EXPECT_EQ(search, j);
        }
    }
}



// This test checks the positive_bwdsearch method for every character
// in the input and d = {1,2} works correctly
TEST_P(BPSPCBlockTreeFixture, positive_bwdsearch_check) {
    std::unordered_set<int> ds = {1, 2};
    for (int d : ds) {
        for (int i = 0; i < input_.length(); ++i) {
            int search = bps_->positive_bwdsearch(i, d);
            int excess = 0;
            int j = i;
            while (true) {
                if (j == 0) break;
                excess += (input_[0] == input_[j]) ? 1 : -1;
                if (excess == -d) break;
                --j;
            }
            EXPECT_EQ(search, j-1);
        }
    }
}

// This test checks the my_positive_bwdsearch method for every character
// in the input and d = {1,2} works correctly
TEST_P(BPSPCBlockTreeFixture, my_positive_bwdsearch_check) {
    std::unordered_set<int> ds = {1, 2};
    for (int d : ds) {
        for (int i = 0; i < input_.length(); ++i) {
            int search = bps_->my_positive_bwdsearch(i, d);
            int excess = 0;
            int j = i;
            while (true) {
                if (j == 0) break;
                excess += (input_[0] == input_[j]) ? 1 : -1;
                if (excess == -d) break;
                --j;
            }
            EXPECT_EQ(search, j-1);
        }
    }
}


// This test checks the my_min_excess(i,j) method for some sample
TEST_P(BPSPCBlockTreeFixture, my_min_excess_check) {
    for (int j = 0; j < input_.length(); j = 2*j+1) {
        for (int i = 0; i <= j; i = 2*i+1) {
            EXPECT_EQ(block_tree_->root_block_->linear_min_excess(i,j), bps_->my_min_excess(i,j));
        }
    }
}

// This test checks the min_excess(i,j) method for some sample
TEST_P(BPSPCBlockTreeFixture, min_excess_check) {
    for (int j = 0; j < input_.length(); j = 2*j+1) {
        for (int i = 0; i <= j; i = 2*i+1) {
            EXPECT_EQ(block_tree_->root_block_->linear_min_excess(i,j), bps_->min_excess(i,j));
        }
    }
}



// This test checks the max_excess(i,j) method for some sample
TEST_P(BPSPCBlockTreeFixture, max_excess_check) {
    for (int j = 0; j < input_.length(); j = 2*j+1) {
        for (int i = 0; i <= j; i = 2*i+1) {
            EXPECT_EQ(block_tree_->root_block_->linear_max_excess(i,j), bps_->max_excess(i,j));
        }
    }
}

// This test checks the my_max_excess(i,j) method for some sample
TEST_P(BPSPCBlockTreeFixture, my_max_excess_check) {
    for (int j = 0; j < input_.length(); j = 2*j+1) {
        for (int i = 0; i <= j; i = 2*i+1) {
            EXPECT_EQ(block_tree_->root_block_->linear_max_excess(i,j), bps_->my_max_excess(i,j));
        }
    }
}

// This test checks the my_min_count(i,j) method for some sample
TEST_P(BPSPCBlockTreeFixture, my_min_count_check) {
    for (int j = 0; j < input_.length(); j = 2*j+1) {
        for (int i = 0; i <= j; i = 2*i+1) {
            EXPECT_EQ(block_tree_->root_block_->linear_min_count(i,j), bps_->my_min_count(i,j));
        }
    }
}


// This test checks the min_count(i,j) method for some sample
TEST_P(BPSPCBlockTreeFixture, min_count_check) {
    for (int j = 0; j < input_.length(); j = 2*j+1) {
        for (int i = 0; i <= j; i = 2*i+1) {
            EXPECT_EQ(block_tree_->root_block_->linear_min_count(i,j), bps_->min_count(i,j));
        }
    }
}

// This test checks the min_select(i,j,t) method for some sample
TEST_P(BPSPCBlockTreeFixture, min_select_check) {
    for (int j = 0; j < input_.length(); j = 2*j+1) {
        for (int i = 0; i <= j; i = 2*i+1) {
            int count = block_tree_->min_count(i,j);
            for (int t = 1; t <= count ; ++t) {
                EXPECT_EQ(block_tree_->root_block_->linear_min_select(i,j,t), bps_->min_select(i,j,t));
            }
        }
    }
}


// This test checks the my_min_select(i,j,t) method for some sample
TEST_P(BPSPCBlockTreeFixture, my_min_select_check) {
    for (int j = 0; j < input_.length(); j = 2*j+1) {
        for (int i = 0; i <= j; i = 2*i+1) {
            int count = block_tree_->min_count(i,j);
            for (int t = 1; t <= count ; ++t) {
                EXPECT_EQ(block_tree_->root_block_->linear_min_select(i,j,t), bps_->my_min_select(i,j,t));
            }
        }
    }
}

// This test checks if the BPSPCBlockTree has the same
// structure that its correspondent PBlockTree
// in particular the bt_first|second_leaf_ranks field are checked
TEST_P(BPSPCBlockTreeFixture, bt_first_second_leaf_ranks_field_check) {
    auto iterator = block_tree_->levelwise_iterator();
    std::vector<PBlock*> level;
    bool contains_back_block = false;
    int i = 0;
    for (; i < iterator.size(); ++i) {
        level = iterator[i];
        for (PBlock *b : level) {
            if (dynamic_cast<PBackBlock*>(b)) contains_back_block = true;
        }
        if (contains_back_block) break;
    }

    for (int j = 0; j < bps_->number_of_levels_-1; ++j) {
        level = iterator[i+j];
        auto level_bt_first_ranks = *(bps_->bt_first_leaf_ranks_[j]);
        auto level_bt_second_ranks = *(bps_->bt_second_leaf_ranks_[j]);

        int l = 0;
        for (PBlock *b: level) {
            if (dynamic_cast<PBackBlock *>(b)) {
                EXPECT_EQ(level_bt_first_ranks[l], b->first_leaf_rank_) ;
                EXPECT_EQ(level_bt_second_ranks[l], b->second_leaf_rank_) ;
                ++l;
            }
        }
        EXPECT_EQ(l, level_bt_first_ranks.size());
        EXPECT_EQ(l, level_bt_second_ranks.size());

    }
}

// This test checks if the BPSCBlockTree has the same
// structure that its correspondent PBlockTree
// in particular the bt_leaf_ranks_, bt_prefix_leaf_ranks_,
// bt_cumulated_leaf_ranks_ are checked
TEST_P(BPSPCBlockTreeFixture, bt_bv_leaf_ranks_prefix_cumulated_check) {
    auto iterator = block_tree_->levelwise_iterator();
    std::vector<PBlock*> level;
    bool contains_back_block = false;
    int i = 0;
    for (; i < iterator.size(); ++i) {
        level = iterator[i];
        for (PBlock *b : level) {
            if (dynamic_cast<PBackBlock*>(b)) contains_back_block = true;
        }
        if (contains_back_block) break;
    }



    level = iterator[i];
    auto level_bt_ranks = *(bps_->bt_leaf_ranks_[0]);
    EXPECT_EQ(level.size(), level_bt_ranks.size());

    for (int k = 0; k < level.size(); ++k) {
        PBlock* b = level[k];
        EXPECT_EQ(b->leaf_rank_, level_bt_ranks[k]);
    }



    for (int j = 1; j < bps_->number_of_levels_; ++j) {
        level = iterator[i + j];
        auto level_bt_ranks = *(bps_->bt_leaf_ranks_[j]);
        auto level_bt_prefix_ranks = *(bps_->bt_prefix_leaf_ranks_[j-1]);
        auto level_bt_cumulated_ranks = *(bps_->bt_cumulated_leaf_ranks_[j-1]) ;
        EXPECT_EQ(level.size(), level_bt_ranks.size());
        EXPECT_EQ(level.size(), level_bt_prefix_ranks.size());
        EXPECT_EQ(level.size(), level_bt_cumulated_ranks.size());

        for (int k = 0; k < level.size(); ++k) {
            PBlock* b = level[k];
            EXPECT_EQ(b->leaf_rank_, level_bt_ranks[k]);
            EXPECT_EQ(b->prefix_leaf_rank_, level_bt_prefix_ranks[k]);
            EXPECT_EQ(b->cumulated_leaf_rank_, level_bt_cumulated_ranks[k]);
        }
    }
}


// This test checks if the BPSCBlockTree has the same
// structure that its correspondent PBlockTree
// in particular the first level for bt_prefix_leaf_ranks_,
// bt_cumulated_leaf_ranks_ are checked
TEST_P(BPSPCBlockTreeFixture, bt_bv_first_level_prefix_cumulated_leaf_ranks_check) {
    auto iterator = block_tree_->levelwise_iterator();
    std::vector<PBlock*> level;
    bool contains_back_block = false;
    int i = 0;
    for (; i < iterator.size(); ++i) {
        level = iterator[i];
        for (PBlock *b : level) {
            if (dynamic_cast<PBackBlock*>(b)) contains_back_block = true;
        }
        if (contains_back_block) break;
    }


    level = iterator[i];
    auto first_level_bt_prefix_ranks = *(bps_->bt_first_level_prefix_leaf_ranks_);
    auto first_level_bt_cumulated_ranks = *(bps_->bt_first_level_cumulated_leaf_ranks_);
    int r  = 0;

    EXPECT_EQ(first_level_bt_prefix_ranks.size(), level.size());
    EXPECT_EQ(first_level_bt_cumulated_ranks.size(), level.size());
    for (int k = 0; k < level.size(); ++k) {
        EXPECT_EQ(r, first_level_bt_prefix_ranks[k]);
        r += level[k]->leaf_rank_;
        EXPECT_EQ(r, first_level_bt_cumulated_ranks[k]);
    }


}


// This test checks if the BPSCBlockTree has the same
// structure that its correspondent PBlockTree
// in particular the bt_starts_with_end_leaf_, bt_suffix_start_with_end_leaf_,
// bt_prefix_start_with_end_leaf_ are checked
TEST_P(BPSPCBlockTreeFixture, bt_bv_border_fields_check) {
    auto iterator = block_tree_->levelwise_iterator();
    std::vector<PBlock*> level;
    bool contains_back_block = false;
    int i = 0;
    for (; i < iterator.size(); ++i) {
        level = iterator[i];
        for (PBlock *b : level) {
            if (dynamic_cast<PBackBlock*>(b)) contains_back_block = true;
        }
        if (contains_back_block) break;
    }



    for (int j = 0; j < bps_->number_of_levels_-1; ++j) {
        level = iterator[i+j];
        auto level_bt_starts_with_end_leaf = *(bps_->bt_starts_with_end_leaf_[j]);
        auto level_bt_suffix_start_with_end_leaf = *(bps_->bt_suffix_start_with_end_leaf_[j]);
        auto level_bt_prefix_start_with_end_leaf = *(bps_->bt_prefix_start_with_end_leaf_[j]);

        EXPECT_EQ(level_bt_starts_with_end_leaf.size(), level.size());
        int l = 0;
        int k = 0;
        for (PBlock *b: level) {
            EXPECT_EQ((b->starts_with_end_leaf_)?1:0, level_bt_starts_with_end_leaf[k]);
            if (dynamic_cast<PBackBlock *>(b)) {
                PBackBlock* bb = dynamic_cast<PBackBlock*>(b);
                EXPECT_EQ(level_bt_suffix_start_with_end_leaf[l], bb->suffix_start_with_end_leaf_) ;
                EXPECT_EQ(level_bt_prefix_start_with_end_leaf[l], bb->prefix_start_with_end_leaf_) ;
                ++l;
            }
            ++k;
        }

        EXPECT_EQ(l, level_bt_suffix_start_with_end_leaf.size());
        EXPECT_EQ(l, level_bt_prefix_start_with_end_leaf.size());

    }

}


//ESTO
// This test checks if the BPSCBlockTree has the same
// structure that its correspondent PBlockTree
// in particular the bt_prefix_min_excess, bt_prefix_max_excess,
// bt_min_count and bt_excess
TEST_P(BPSPCBlockTreeFixture, bt_prefix_min_max_excess_count_check) {
    auto iterator = block_tree_->levelwise_iterator();
    std::vector<PBlock*> level;
    bool contains_back_block = false;
    int i = 0;
    for (; i < iterator.size(); ++i) {
        level = iterator[i];
        for (PBlock *b : level) {
            if (dynamic_cast<PBackBlock*>(b)) contains_back_block = true;
        }
        if (contains_back_block) break;
    }


    for (int j = i; j < iterator.size(); ++j) {
        level = iterator[j];
        sdsl::int_vector<> excess_level = (*bps_->bt_excess_[j-i]);
        sdsl::int_vector<> prefix_min_level = (*bps_->bt_prefix_min_excess_[j-i]);
        sdsl::int_vector<> prefix_max_level = (*bps_->bt_prefix_max_excess_[j-i]);
        sdsl::int_vector<> min_count_level = (*bps_->bt_min_count_[j-i]);
        EXPECT_EQ(level.size(), prefix_min_level.size());
        EXPECT_EQ(level.size(), prefix_max_level.size());
        EXPECT_EQ(level.size(), min_count_level.size());
        for (int k = 0; k < level.size(); ++k) {
            EXPECT_EQ(level[k]->min_prefix_excess_, 1-prefix_min_level[k]);
            EXPECT_EQ(level[k]->max_prefix_excess_, -1+prefix_max_level[k]);
            EXPECT_EQ(level[k]->prefix_min_count_, min_count_level[k]);
            int excess = 0;
            for (auto pair : level[k]->ranks_) {
                if (pair.first == bps_->open_) excess += pair.second;
                else excess -= pair.second;
            }
            EXPECT_EQ(excess, (excess_level[k]%2) ? -(excess_level[k]/2) : excess_level[k]/2);
        }
    }

}
/*
//ESTO
// This test checks if the BPSCBlockTree has the same
// structure that its correspondent PBlockTree
// in particular the first level for bt_prefix_min_excess,
// bt_prefix_max_excess
TEST_P(BPSPCBlockTreeFixture, bt_first_level_prefix_min_max_excess_check) {
    auto iterator = block_tree_->levelwise_iterator();
    std::vector<PBlock*> level;
    bool contains_back_block = false;
    int i = 0;
    for (; i < iterator.size(); ++i) {
        level = iterator[i];
        for (PBlock *b : level) {
            if (dynamic_cast<PBackBlock*>(b)) contains_back_block = true;
        }
        if (contains_back_block) break;
    }


    level = iterator[i];
    auto first_level_prefix_min = *(bps_->bt_first_level_prefix_min_excess_);
    auto first_level_prefix_max = *(bps_->bt_first_level_prefix_max_excess_);
    int excess  = 0;

    EXPECT_EQ(first_level_prefix_min.size(), level.size());
    EXPECT_EQ(first_level_prefix_max.size(), level.size());
    for (int k = 0; k < level.size(); ++k) {
        EXPECT_EQ(excess+(level[k]->min_prefix_excess_), first_level_prefix_min[k]);
        EXPECT_EQ(excess+(level[k]->max_prefix_excess_), first_level_prefix_max[k]);
        for (auto pair : level[k]->ranks_) {
            if (pair.first == input_[0]) excess += pair.second;
            else excess -= pair.second;
        }

    }
}
*/

// ESTOP
// This test checks if the BPSCBlockTree has the same
// structure that its correspondent PBlockTree
// in particular the bt_excess, bt_prefix_min_excess, bt_prefix_max_excess,
// bt_min_count for the first and second blocks!
TEST_P(BPSPCBlockTreeFixture, bt_prefix_min_max_excess_count_first_and_second_block_check) { //HERE!
    auto iterator = block_tree_->levelwise_iterator();
    std::vector<PBlock*> level;
    bool contains_back_block = false;
    int i = 0;
    for (; i < iterator.size(); ++i) {
        level = iterator[i];
        for (PBlock *b : level) {
            if (dynamic_cast<PBackBlock*>(b)) contains_back_block = true;
        }
        if (contains_back_block) break;
    }


    for (int j = i; j < iterator.size()-1; ++j) {
        level = iterator[j];
        sdsl::int_vector<> min_count_first_block_level = (*bps_->bt_min_count_first_block_[j-i]);
        sdsl::int_vector<> min_count_second_block_level = (*bps_->bt_min_count_second_block_[j-i]);

        sdsl::int_vector<> excess_first_block_level = (*bps_->bt_excess_first_block_[j-i]);
        sdsl::int_vector<> excess_second_block_level = (*bps_->bt_excess_second_block_[j-i]);

        sdsl::int_vector<> prefix_min_first_block_level = (*bps_->bt_prefix_min_excess_first_block_[j-i]);
        sdsl::int_vector<> prefix_min_second_block_level = (*bps_->bt_prefix_min_excess_second_block_[j-i]);

        sdsl::int_vector<> prefix_max_first_block_level = (*bps_->bt_prefix_max_excess_first_block_[j-i]);
        sdsl::int_vector<> prefix_max_second_block_level = (*bps_->bt_prefix_max_excess_second_block_[j-i]);


        int b = 0;
        for (int k = 0; k < level.size(); ++k) {
            if (dynamic_cast<PBackBlock*>(level[k])) {
                PBackBlock* bb = dynamic_cast<PBackBlock*>(level[k]);
                EXPECT_EQ(bb->prefix_min_first_block_count_, min_count_first_block_level[b]);
                EXPECT_EQ(bb->prefix_min_second_block_count_, min_count_second_block_level[b]);

                EXPECT_EQ(bb->prefix_first_block_min_excess_, 1-prefix_min_first_block_level[b]);
                EXPECT_EQ(bb->prefix_second_block_min_excess_, 1-prefix_min_second_block_level[b]);

                EXPECT_EQ(bb->prefix_first_block_max_excess_, -1+prefix_max_first_block_level[b]);
                EXPECT_EQ(bb->prefix_second_block_max_excess_, -1+prefix_max_second_block_level[b]);

                int first_block_excess = 0;
                for (auto pair : bb->second_ranks_) {
                    if (pair.first == bps_->open_) first_block_excess += pair.second;
                    else first_block_excess -= pair.second;
                }
                int second_block_excess = -first_block_excess;
                for (auto pair : bb->ranks_) {
                    if (pair.first == bps_->open_) second_block_excess += pair.second;
                    else second_block_excess -= pair.second;
                }

                EXPECT_EQ(first_block_excess, (excess_first_block_level[b]%2) ? -(excess_first_block_level[b]/2) : excess_first_block_level[b]/2);
                EXPECT_EQ(second_block_excess, (excess_second_block_level[b]%2) ? -(excess_second_block_level[b]/2) : excess_second_block_level[b]/2);
                ++b;
            }
        }
        EXPECT_EQ(min_count_first_block_level.size(), b);
        EXPECT_EQ(min_count_second_block_level.size(), b);
    }

}

/*
// This test checks if the BPSCBlockTree has the same
// structure that its correspondent PBlockTree
// in particular the information excess, min ,max and count
// for the deleted nodes is correct
TEST_P(BPSPCBlockTreeFixture, bt_deleted_nodes_check) {
    auto iterator = block_tree_->levelwise_iterator();
    std::vector<PBlock*> level;
    bool contains_back_block = false;
    int i = 0;
    int deleted_levels = 0;
    for (; i < iterator.size(); ++i) {
        level = iterator[i];
        for (PBlock *b : level) {
            if (dynamic_cast<PBackBlock*>(b)) contains_back_block = true;
        }
        if (contains_back_block) break;
        ++deleted_levels;
    }

    int j = 0;
    for (int dl = 0; dl < deleted_levels; dl++) {
        for (PBlock* b : iterator[dl]) {
            int excess = 0;
            for (auto pair : b->ranks_) {
                if (pair.first == input_[0]) excess += pair.second;
                else excess -= pair.second;
            }
            EXPECT_EQ(excess, (*bps_->bt_deleted_excess_)[j]);
            EXPECT_EQ(b->min_prefix_excess_, (*bps_->bt_deleted_min_excess_)[j]);
            EXPECT_EQ(b->max_prefix_excess_, (*bps_->bt_deleted_max_excess_)[j]);
            EXPECT_EQ(b->prefix_min_count_, (*bps_->bt_deleted_min_count_)[j]);
            ++j;
        }
    }
}
*/
// This test checks if the BPSCBlockTree has the same
// structure that its correspondent PBlockTree
// in particular the information excess, min ,max and count
// for the top structure is correct besides the n_last_level_ and
// n_pre_last_level_ fields
TEST_P(BPSPCBlockTreeFixture, bt_top_structures_check) {
    int n = bps_->number_of_blocks_first_level_;
    int i = std::log(n)/std::log(r_);
    int r_pow_i = std::pow(r_, i);
    int p = (n-r_pow_i)/(r_-1);
    int m = (n-r_pow_i)%(r_-1);
    if (m>0) ++p;
    EXPECT_EQ(bps_->n_pre_last_level_, r_pow_i-p);
    EXPECT_EQ(bps_->n_last_level_, n+p-r_pow_i);
    int nodes = (r_pow_i*r_ - 1)/(r_-1) - bps_->n_pre_last_level_;
    EXPECT_EQ(nodes, bps_->top_excess_->size());
    EXPECT_EQ(nodes, bps_->top_min_excess_->size());
    EXPECT_EQ(nodes, bps_->top_max_excess_->size());
    EXPECT_EQ(nodes, bps_->top_min_count_->size());

    for (int i = 0; i < nodes; ++i) {
        int excess = decoded_excess((*bps_->top_excess_)[i]);
        for (int child = r_*i+1; child < r_*i+r_+1; ++child) {
            if (child >= nodes + bps_->n_last_level_ + bps_->n_pre_last_level_) break;
            if (child < nodes) {
                excess -= decoded_excess((*bps_->top_excess_)[child]);
            } else {
                int child_index = 0;
                if (child - nodes < bps_->n_pre_last_level_) child_index = child-nodes+bps_->n_last_level_;
                else child_index = child-nodes-bps_->n_pre_last_level_;
                excess -= decoded_excess((*(bps_->bt_excess_[0]))[child_index]);
            }
        }
        EXPECT_EQ(excess,0);
    }

    for (int i = 0; i < nodes; ++i) {
        int min_excess = 1-(*bps_->top_min_excess_)[i];
        int cumulated_excess = 0;
        for (int child = r_*i+1; child < r_*i+r_+1; ++child) {
            if (child >= nodes + bps_->n_last_level_ + bps_->n_pre_last_level_) break;
            if (child < nodes) {
                EXPECT_LE(min_excess, cumulated_excess + 1-((int)((*bps_->top_min_excess_)[child])));
                cumulated_excess += ((*bps_->top_excess_)[child]%2) ? -((*bps_->top_excess_)[child]/2) : (*bps_->top_excess_)[child]/2;;
            } else {
                int child_index = 0;
                if (child - nodes < bps_->n_pre_last_level_) child_index = child-nodes+bps_->n_last_level_;
                else child_index = child-nodes-bps_->n_pre_last_level_;

                EXPECT_LE(min_excess, cumulated_excess + 1-((int)((*bps_->bt_prefix_min_excess_[0])[child_index])));
                cumulated_excess += decoded_excess((*(bps_->bt_excess_[0]))[child_index]);
            }

        }
    }


    for (int i = 0; i < nodes; ++i) {
        int max_excess = -1+(*bps_->top_max_excess_)[i];
        int cumulated_excess = 0;
        for (int child = r_*i+1; child < r_*i+r_+1; ++child) {
            if (child >= nodes + bps_->n_last_level_ + bps_->n_pre_last_level_) break;
            if (child < nodes) {
                EXPECT_GE(max_excess, cumulated_excess + -1+(int)((*bps_->top_max_excess_)[child]));
                cumulated_excess += ((*bps_->top_excess_)[child]%2) ? -((*bps_->top_excess_)[child]/2) : (*bps_->top_excess_)[child]/2;;
            } else {
                int child_index = 0;
                if (child - nodes < bps_->n_pre_last_level_) child_index = child-nodes+bps_->n_last_level_;
                else child_index = child-nodes-bps_->n_pre_last_level_;

                EXPECT_GE(max_excess, cumulated_excess + -1+(int)((*bps_->bt_prefix_max_excess_[0])[child_index]));
                cumulated_excess += decoded_excess((*(bps_->bt_excess_[0]))[child_index]);
            }

        }
    }


    for (int i = 0; i < nodes; ++i) {
        int min_excess = 1-(*bps_->top_min_excess_)[i];
        int cumulated_excess = 0;
        int min_count = 0;
        for (int child = r_*i+1; child < r_*i+r_+1; ++child) {
            if (child >= nodes + bps_->n_last_level_ + bps_->n_pre_last_level_) break;
            if (child < nodes) {
                if (min_excess == cumulated_excess + 1-((int)((*bps_->top_min_excess_)[child]))) min_count += (*bps_->top_min_count_)[child];
                cumulated_excess += ((*bps_->top_excess_)[child]%2) ? -((*bps_->top_excess_)[child]/2) : (*bps_->top_excess_)[child]/2;;
            } else {
                int child_index = 0;
                if (child - nodes < bps_->n_pre_last_level_) child_index = child-nodes+bps_->n_last_level_;
                else child_index = child-nodes-bps_->n_pre_last_level_;

                if (min_excess == cumulated_excess + 1-((int)((*bps_->bt_prefix_min_excess_[0])[child_index]))) min_count += (*bps_->bt_min_count_[0])[child_index];
                cumulated_excess += decoded_excess((*(bps_->bt_excess_[0]))[child_index]);
            }

        }
        EXPECT_EQ(min_count, (*bps_->top_min_count_)[i]);
    }
}