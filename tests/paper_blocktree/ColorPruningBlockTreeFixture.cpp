//
// Created by sking32 on 6/26/18.
//

#include <paper_blocktree/LeafBlock.h>
#include <paper_blocktree/LazyInternalBlock.h>
#include <paper_blocktree/InternalBlock.h>
#include <unordered_set>
#include <fstream>
#include "gtest/gtest.h"

#include "paper_blocktree/BlockTree.h"

using ::testing::Combine;
using ::testing::Values;

class ColorPruningBlockTreeFixture : public ::testing::TestWithParam<::testing::tuple<int, int, std::string, int>> {
protected:
    virtual void TearDown() {
    }

    virtual void SetUp() {
    }

public:
    BlockTree* block_tree_;
    //BlockTree* block_tree_rs_;

    std::string input_;
    int r_;
    int max_leaf_length_;
    int c_;
    std::unordered_map<int,std::vector<int>> characters_; // Characters in the input and its select results

    ColorPruningBlockTreeFixture() : ::testing::TestWithParam<::testing::tuple<int, int, std::string, int>>() {
        r_ = ::testing::get<0>(GetParam());
        max_leaf_length_ = ::testing::get<1>(GetParam());
        c_ = ::testing::get<3>(GetParam());

        std::ifstream t(::testing::get<2>(GetParam()));
        std::stringstream buffer;
        buffer << t.rdbuf();
        input_= buffer.str();
        block_tree_ = new BlockTree(input_, r_ , max_leaf_length_);
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

    virtual ~ColorPruningBlockTreeFixture() {
        delete block_tree_;
//        delete block_tree_rs_;
    }
};

INSTANTIATE_TEST_CASE_P(BlockTreeConstruction,
                        ColorPruningBlockTreeFixture,
                        Combine(Values(2),
                                Values(4),
                                Values("../../tests/data/as", "../../tests/data/dna", "../../tests/data/dna.par"/*, "../../tests/data/einstein"*/),
                                Values(1,2,4,8)));


// This test checks that max_hop isn't greater than the color
TEST_P(ColorPruningBlockTreeFixture, color_property_check) {
    EXPECT_LE(block_tree_->get_max_hop(), c_);
}


// This test checks that back blocks don't point to themselves
TEST_P(ColorPruningBlockTreeFixture, no_self_references_check) {
    std::vector<Block*> level = {block_tree_->root_block_};
    for (std::vector<Block*> level : block_tree_->levelwise_iterator()) {
        for (Block* b: level) {
            if (dynamic_cast<BackBlock*>(b)) {
                EXPECT_NE(b->first_block_, b);
                if (b->second_block_ != nullptr) {
                    EXPECT_NE(b->second_block_, b);
                }
            }
        }
    }
}

// This test checks if pointed blocks are consecutive
TEST_P(ColorPruningBlockTreeFixture, text_consecutive_pointed_blocks_check) {
    std::vector<Block*> level = {block_tree_->root_block_};
    for (std::vector<Block*> level : block_tree_->levelwise_iterator()) {
        for (Block* b: level) {
            if (b->second_block_ != nullptr  && dynamic_cast<BackBlock*>(b)) {
                EXPECT_EQ(b->first_block_->end_index_, b->second_block_->start_index_-1);
            }
        }
    }
}


// This test checks whether the back blocks points to first
// occurrences on the input string
TEST_P(ColorPruningBlockTreeFixture, pointing_to_first_occurrence_check) {
    std::vector<Block*> level = {block_tree_->root_block_};
    for (std::vector<Block*> level : block_tree_->levelwise_iterator()) {
        for (Block* b: level) {
            if (dynamic_cast<BackBlock*>(b)) {
                int i = input_.find(b->represented_string());
                EXPECT_EQ(b->first_block_->start_index_ + b->offset_ , i) << b->start_index_ << "-" << b->end_index_;
            }
        }
    }
}

// This test checks if the left and right flags are
// correctly set
TEST_P(ColorPruningBlockTreeFixture, left_right_field_check) {
    std::vector<Block*> level = {block_tree_->root_block_};
    for (std::vector<Block*> level : block_tree_->levelwise_iterator()) {
        for (Block* b: level) {
            if (dynamic_cast<BackBlock*>(b)) {
                EXPECT_TRUE(b->left_ && b->right_);
            } else {
                EXPECT_FALSE(b->left_ && b->right_);
            }
        }
    }
}


// This test checks if the NO back pointer
// doesn't have reason to be there
TEST_P(ColorPruningBlockTreeFixture, no_back_pointer_check) {
    std::vector<Block*> level = {block_tree_->root_block_};
    for (std::vector<Block*> level : block_tree_->levelwise_iterator()) {
        for (int i = 0; i < level.size(); ++i) {
            Block* b = level[i];
            if (dynamic_cast<InternalBlock*>(b) || dynamic_cast<LazyInternalBlock*>(b)) {
                int index = input_.find(b->represented_string());
                if (index < b->start_index_) {
                    bool check = false;
                    check |= (i != 0  && (input_.find(level[i-1]->represented_string()+b->represented_string()))== level[i-1]->start_index_);
                    check |= (i != level.size()-1  && (input_.find(b->represented_string()+level[i+1]->represented_string()))== b->start_index_);
                    EXPECT_TRUE(check);
                }
            }
        }
    }
}


bool pointing_to_subtree(Block* b) {
    std::vector<Block*> level = b->children_;
    while (level.size() != 0) {
        std::vector<Block*> next_level;
        for (Block* b : level) {
            if (b->pointing_to_me_ != 0) return true;
            for (Block *child : b->children_)
                next_level.push_back(child);
        }
        level = next_level;
    }
    return false;
}

// This test checks if there is no unnecessary expansions
TEST_P(ColorPruningBlockTreeFixture, no_unnecessary_expansions_check) {
    std::vector<Block*> level = {block_tree_->root_block_};
    for (std::vector<Block*> level : block_tree_->levelwise_iterator()) {
        for (Block* b: level) {
            if (dynamic_cast<InternalBlock*>(b) || dynamic_cast<LazyInternalBlock*>(b)) {
                EXPECT_TRUE(b->color_ >= c_ || b->first_occurrence_level_index_ == b->level_index_ ||
                            pointing_to_subtree(b));
            }
        }
    }
}