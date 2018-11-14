
#include <padding_blocktree/PLeafBlock.h>
#include <padding_blocktree/PLazyInternalBlock.h>
#include <padding_blocktree/PInternalBlock.h>
#include <unordered_set>
#include <fstream>
#include "gtest/gtest.h"

#include "padding_blocktree/PBlockTree.h"

using ::testing::Combine;
using ::testing::Values;

class PRealColorBlockTreeFixture : public ::testing::TestWithParam<::testing::tuple<int, int, std::string, int>> {
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

    PRealColorBlockTreeFixture() : ::testing::TestWithParam<::testing::tuple<int, int, std::string, int>>() {
        r_ = ::testing::get<0>(GetParam());
        max_leaf_length_ = ::testing::get<1>(GetParam());
        c_ = ::testing::get<3>(GetParam());

        std::ifstream t(::testing::get<2>(GetParam()));
        std::stringstream buffer;
        buffer << t.rdbuf();
        input_= buffer.str();
        block_tree_ = new PBlockTree(input_, r_ , max_leaf_length_);
        block_tree_->process_back_pointers_real_conservative_heuristic(c_);

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

    virtual ~PRealColorBlockTreeFixture() {
        delete block_tree_;
//        delete block_tree_rs_;
    }
};

INSTANTIATE_TEST_CASE_P(BlockTreeConstruction,
                        PRealColorBlockTreeFixture,
                        Combine(Values(2),
                                Values(4),
                                Values("../../tests/data/as", "../../tests/data/dna", "../../tests/data/dna.par"/*, "../../tests/data/einstein"*/),
                                Values(1,2,4,8)));


// This test checks that max_hop isn't greater than the color
TEST_P(PRealColorBlockTreeFixture, color_property_check) {
    EXPECT_LE(block_tree_->get_max_hop(), c_);
}


// This test checks that back blocks don't point to themselves
TEST_P(PRealColorBlockTreeFixture, no_self_references_check) {
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
TEST_P(PRealColorBlockTreeFixture, text_consecutive_pointed_blocks_check) {
    std::vector<PBlock*> level = {block_tree_->root_block_};
    for (std::vector<PBlock*> level : block_tree_->levelwise_iterator()) {
        for (PBlock* b: level) {
            if (b->second_block_ != nullptr  && dynamic_cast<PBackBlock*>(b)) {
                EXPECT_EQ(b->first_block_->end_index_, b->second_block_->start_index_-1);
            }
        }
    }
}