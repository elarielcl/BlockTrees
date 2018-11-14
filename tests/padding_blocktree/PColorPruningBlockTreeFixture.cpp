//
// Created by sking32 on 6/26/18.
//

//
// Created by sking32 on 6/26/18.
//

#include <padding_blocktree/PLeafBlock.h>
#include <padding_blocktree/PLazyInternalBlock.h>
#include <padding_blocktree/PInternalBlock.h>
#include <unordered_set>
#include <fstream>
#include "gtest/gtest.h"

#include "padding_blocktree/PBlockTree.h"

using ::testing::Combine;
using ::testing::Values;

class PColorPruningBlockTreeFixture : public ::testing::TestWithParam<::testing::tuple<int, int, std::string, int>> {
protected:
    virtual void TearDown() {
    }

    virtual void SetUp() {
    }

public:
    PBlockTree* block_tree_;
    //BlockTree* block_tree_rs_;

    std::string input_;
    int r_;
    int max_leaf_length_;
    int c_;
    std::unordered_map<int,std::vector<int>> characters_; // Characters in the input and its select results

    PColorPruningBlockTreeFixture() : ::testing::TestWithParam<::testing::tuple<int, int, std::string, int>>() {
        r_ = ::testing::get<0>(GetParam());
        max_leaf_length_ = ::testing::get<1>(GetParam());
        c_ = ::testing::get<3>(GetParam());

        std::ifstream t(::testing::get<2>(GetParam()));
        std::stringstream buffer;
        buffer << t.rdbuf();
        input_= buffer.str();
        block_tree_ = new PBlockTree(input_, r_ , max_leaf_length_);
        block_tree_->process_back_pointers();
        block_tree_->clean_unnecessary_expansions(c_);

        /*
        block_tree_rs_ = new BlockTree(input_, r_ , max_leaf_length_);
        block_tree_rs_->process_back_pointers();
        block_tree_rs_->clean_unnecessary_expansions();

        std::unordered_set<int> characters;
        for (char c: input_)
            characters.insert(c);
        for (int c: characters) {
            characters_[c] = {};
            block_tree_rs_->add_rank_select_support(c);
        }

        for (int i = 0; i<input_.size(); ++i)
            characters_[input_[i]].push_back(i);
        */
    }

    virtual ~PColorPruningBlockTreeFixture() {
        delete block_tree_;
//        delete block_tree_rs_;
    }
};

INSTANTIATE_TEST_CASE_P(PBlockTreeConstruction,
                        PColorPruningBlockTreeFixture,
                        Combine(Values(2),
                                Values(4),
                                Values("../../tests/data/as", "../../tests/data/dna", "../../tests/data/dna.par"/*, "../../tests/data/einstein"*/),
                                Values(1,2,4,8)));


// This test checks that max_hop isn't greater than the color
TEST_P(PColorPruningBlockTreeFixture, color_property_check) {
    EXPECT_LE(block_tree_->get_max_hop(), c_);
}


// This test checks that back blocks don't point to themselves
TEST_P(PColorPruningBlockTreeFixture, no_self_references_check) {
    std::vector<PBlock*> level = {block_tree_->root_block_};
    for (std::vector<PBlock*> level : block_tree_->levelwise_iterator()) {
        for (PBlock* b: level) {
            if (dynamic_cast<PBackBlock*>(b)) {
                EXPECT_NE(b->first_block_, b);
                if (b->second_block_ != nullptr) {
                    EXPECT_NE(b->second_block_, b);
                }
            }
        }
    }
}

// This test checks if pointed blocks are consecutive
TEST_P(PColorPruningBlockTreeFixture, text_consecutive_pointed_blocks_check) {
    std::vector<PBlock*> level = {block_tree_->root_block_};
    for (std::vector<PBlock*> level : block_tree_->levelwise_iterator()) {
        for (PBlock* b: level) {
            if (b->second_block_ != nullptr  && dynamic_cast<PBackBlock*>(b)) {
                EXPECT_EQ(b->first_block_->end_index_, b->second_block_->start_index_-1);
            }
        }
    }
}


// This test checks whether the back blocks points to first
// occurrences on the input string
TEST_P(PColorPruningBlockTreeFixture, pointing_to_first_occurrence_check) {
    std::vector<PBlock*> level = {block_tree_->root_block_};
    for (std::vector<PBlock*> level : block_tree_->levelwise_iterator()) {
        for (PBlock* b: level) {
            if (dynamic_cast<PBackBlock*>(b)) {
                int i = input_.find(b->represented_string());
                EXPECT_EQ(b->first_block_->start_index_ + b->offset_ , i) << b->start_index_ << "-" << b->end_index_;
            }
        }
    }
}

// This test checks if the left and right flags are
// correctly set
TEST_P(PColorPruningBlockTreeFixture, left_right_field_check) {
    std::vector<PBlock*> level = {block_tree_->root_block_};
    for (std::vector<PBlock*> level : block_tree_->levelwise_iterator()) {
        for (PBlock* b: level) {
            if (dynamic_cast<PBackBlock*>(b)) {
                EXPECT_TRUE(b->left_ && b->right_);
            } else {
                EXPECT_FALSE(b->left_ && b->right_);
            }
        }
    }
}


// This test checks if the NO back pointer
// doesn't have reason to be there
TEST_P(PColorPruningBlockTreeFixture, no_back_pointer_check) {
    std::vector<PBlock*> level = {block_tree_->root_block_};
    for (std::vector<PBlock*> level : block_tree_->levelwise_iterator()) {
        for (int i = 0; i < level.size(); ++i) {
            PBlock* b = level[i];
            if (dynamic_cast<PInternalBlock*>(b) || dynamic_cast<PLazyInternalBlock*>(b)) {
                int index = input_.find(b->represented_string());
                if (index < b->start_index_) {
                    bool check = false;
                    check |= (i != 0  && (input_.find(level[i-1]->represented_string()+b->represented_string()))== level[i-1]->start_index_);
                    check |= (i != level.size()-1  && (input_.find(b->represented_string()+level[i+1]->represented_string()))== b->start_index_);
                    check |=  (b->end_index_>= input_.size() && i == level.size()-1);
                    EXPECT_TRUE(check) << i << "."  << level.size();
                }
            }
        }
    }
}


bool pointing_to_subtree(PBlock* b) {
    std::vector<PBlock*> level = b->children_;
    while (level.size() != 0) {
        std::vector<PBlock*> next_level;
        for (PBlock* b : level) {
            if (b->pointing_to_me_ != 0) return true;
            for (PBlock *child : b->children_)
                next_level.push_back(child);
        }
        level = next_level;
    }
    return false;
}

// This test checks if there is no unnecessary expansions
TEST_P(PColorPruningBlockTreeFixture, no_unnecessary_expansions_check) {
    std::vector<PBlock*> level = {block_tree_->root_block_};
    for (std::vector<PBlock*> level : block_tree_->levelwise_iterator()) {
        for (PBlock* b: level) {
            if (dynamic_cast<PInternalBlock*>(b) || dynamic_cast<PLazyInternalBlock*>(b)) {
                EXPECT_TRUE(b->color_ >= c_ || b->first_occurrence_level_index_ == b->level_index_ ||
                            pointing_to_subtree(b)) << b->start_index_ << "-" << b->end_index_;
            }
        }
    }
}