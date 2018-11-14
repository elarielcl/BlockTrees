//
// Created by sking32 on 5/28/18.
//

//
// Created by sking32 on 5/22/18.
//

//
// Created by sking32 on 3/11/18.
//

#include <paper_blocktree/LeafBlock.h>
#include <paper_blocktree/LazyInternalBlock.h>
#include <paper_blocktree/InternalBlock.h>
#include <unordered_set>
#include <fstream>
#include <compressed/CBlockTree.h>
#include <compressed/BPSCBlockTree.h>
#include "gtest/gtest.h"

#include "paper_blocktree/BlockTree.h"

using ::testing::Combine;
using ::testing::Values;

typedef BlockTree* CreateBlockTreeFunc(int, int, std::string);

BlockTree* block_tree(int r, int max_leaf_length, std::string input) {

    BlockTree* block_tree_ = new BlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers();
    block_tree_->clean_unnecessary_expansions();
    return block_tree_;
}

BlockTree* block_tree_without_cleanning(int r, int max_leaf_length, std::string input) {
    BlockTree* block_tree_ = new BlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers();
    return block_tree_;
}

BlockTree* heuristic_concatenate_block_tree(int r, int max_leaf_length, std::string input) {
    BlockTree* block_tree_ = new BlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers_heuristic_concatenate();
    return block_tree_;
}


BlockTree* heuristic_block_tree(int r, int max_leaf_length, std::string input) {
    BlockTree* block_tree_ = new BlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers_heuristic();
    return block_tree_;
}



BlockTree* liberal_heuristic_block_tree(int r, int max_leaf_length, std::string input) {
    BlockTree* block_tree_ = new BlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers_liberal_heuristic();
    return block_tree_;
}


BlockTree* conservative_heuristic_block_tree(int r, int max_leaf_length, std::string input) {
    BlockTree* block_tree_ = new BlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers_conservative_heuristic();
    return block_tree_;
}


BlockTree* conservative_heuristic_3_block_tree(int r, int max_leaf_length, std::string input) {
    BlockTree* block_tree_ = new BlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers_conservative_heuristic(3);
    return block_tree_;
}


BlockTree* conservative_optimized_heuristic_block_tree(int r, int max_leaf_length, std::string input) {
    BlockTree* block_tree_ = new BlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers_conservative_optimized_heuristic();
    return block_tree_;
}

class BPSCBlockTreeFixture : public ::testing::TestWithParam<::testing::tuple<int, int, std::string, CreateBlockTreeFunc*>> {
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

        bps_ = new BPSCBlockTree(block_tree_);
    }

public:
    BlockTree* block_tree_;

    BPSCBlockTree* bps_;


    std::string input_;
    int r_;
    int max_leaf_length_;

    BPSCBlockTreeFixture() : ::testing::TestWithParam<::testing::tuple<int, int, std::string, CreateBlockTreeFunc*>>() {
    }

    virtual ~BPSCBlockTreeFixture() {
    }
};

INSTANTIATE_TEST_CASE_P(BPSCBlockTreeTest,
                        BPSCBlockTreeFixture,
                        Combine(Values(2),
                                Values(4),
                                Values("../../tests/data/dna.par"/*, "../../tests/data/einstein"*/),
                                Values(&block_tree, &block_tree_without_cleanning, &heuristic_concatenate_block_tree, &heuristic_block_tree,
                                       &liberal_heuristic_block_tree, &conservative_heuristic_block_tree, &conservative_heuristic_3_block_tree, &conservative_optimized_heuristic_block_tree)));


// This test checks the excess method for every character
// and position in the input
TEST_P(BPSCBlockTreeFixture, excess_check) {
    int e = 0;
    for (int i = 0; i < input_.length(); ++i) {
        e += (input_[i] == '(') ? 1 : -1;
        EXPECT_EQ(bps_->excess(i), e);
    }
}


// This test checks the rank_10 method for every
// position in the input
TEST_P(BPSCBlockTreeFixture, rank_10_check) {
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
TEST_P(BPSCBlockTreeFixture, rank_10_alternative_check) {
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
TEST_P(BPSCBlockTreeFixture, better_rank_10_check) {
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
TEST_P(BPSCBlockTreeFixture, select_10_check) {
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
TEST_P(BPSCBlockTreeFixture, select_10_alternative_check) {
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
TEST_P(BPSCBlockTreeFixture, better_select_10_check) {
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

// This test checks if the BPSPCBlockTree has the same
// structure that its correspondent PBlockTree
// in particular the bt_first|second_leaf_ranks field are checked
TEST_P(BPSCBlockTreeFixture, bt_first_second_leaf_ranks_field_check) {
    auto iterator = block_tree_->levelwise_iterator();
    std::vector<Block*> level;
    bool contains_back_block = false;
    int i = 0;
    for (; i < iterator.size(); ++i) {
        level = iterator[i];
        for (Block *b : level) {
            if (dynamic_cast<BackBlock*>(b)) contains_back_block = true;
        }
        if (contains_back_block) break;
    }

    for (int j = 0; j < bps_->number_of_levels_-1; ++j) {
        level = iterator[i+j];
        auto level_bt_first_ranks = *(bps_->bt_first_leaf_ranks_[j]);
        auto level_bt_second_ranks = *(bps_->bt_second_leaf_ranks_[j]);

        int l = 0;
        for (Block *b: level) {
            if (dynamic_cast<BackBlock *>(b)) {
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
TEST_P(BPSCBlockTreeFixture, bt_bv_leaf_ranks_prefix_cumulated_check) {
    auto iterator = block_tree_->levelwise_iterator();
    std::vector<Block*> level;
    bool contains_back_block = false;
    int i = 0;
    for (; i < iterator.size(); ++i) {
        level = iterator[i];
        for (Block *b : level) {
            if (dynamic_cast<BackBlock*>(b)) contains_back_block = true;
        }
        if (contains_back_block) break;
    }



    level = iterator[i];
    auto level_bt_ranks = *(bps_->bt_leaf_ranks_[0]);
    EXPECT_EQ(level.size(), level_bt_ranks.size());

    for (int k = 0; k < level.size(); ++k) {
        Block* b = level[k];
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
            Block* b = level[k];
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
TEST_P(BPSCBlockTreeFixture, bt_bv_first_level_prefix_cumulated_leaf_ranks_check) {
    auto iterator = block_tree_->levelwise_iterator();
    std::vector<Block*> level;
    bool contains_back_block = false;
    int i = 0;
    for (; i < iterator.size(); ++i) {
        level = iterator[i];
        for (Block *b : level) {
            if (dynamic_cast<BackBlock*>(b)) contains_back_block = true;
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
TEST_P(BPSCBlockTreeFixture, bt_bv_border_fields_check) {
    auto iterator = block_tree_->levelwise_iterator();
    std::vector<Block*> level;
    bool contains_back_block = false;
    int i = 0;
    for (; i < iterator.size(); ++i) {
        level = iterator[i];
        for (Block *b : level) {
            if (dynamic_cast<BackBlock*>(b)) contains_back_block = true;
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
        for (Block *b: level) {
            EXPECT_EQ((b->starts_with_end_leaf_)?1:0, level_bt_starts_with_end_leaf[k]);
            if (dynamic_cast<BackBlock *>(b)) {
                BackBlock* bb = dynamic_cast<BackBlock*>(b);
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

