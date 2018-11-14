//
// Created by sking32 on 5/22/18.
//

//
// Created by sking32 on 5/22/18.
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

class HeuristicConcatenateBlockTreeFixture : public ::testing::TestWithParam<::testing::tuple<int, int, std::string>> {
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
    std::unordered_map<int,std::vector<int>> characters_; // Characters in the input and its select results

    HeuristicConcatenateBlockTreeFixture() : ::testing::TestWithParam<::testing::tuple<int, int, std::string>>() {
        r_ = ::testing::get<0>(GetParam());
        max_leaf_length_ = ::testing::get<1>(GetParam());

        std::ifstream t(::testing::get<2>(GetParam()));
        std::stringstream buffer;
        buffer << t.rdbuf();
        input_= buffer.str();
        block_tree_ = new BlockTree(input_, r_ , max_leaf_length_);
        block_tree_->process_back_pointers_heuristic_concatenate();

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

    virtual ~HeuristicConcatenateBlockTreeFixture() {
        delete block_tree_;
//        delete block_tree_rs_;
    }
};

INSTANTIATE_TEST_CASE_P(BlockTreeConstruction,
                        HeuristicConcatenateBlockTreeFixture,
                        Combine(Values(2),
                                Values(4),
                                Values("../../tests/data/as", "../../tests/data/dna", "../../tests/data/dna.par"/*, "../../tests/data/einstein"*/)));

// This test checks if back blocks are pointing to its first occurrence
TEST_P(HeuristicConcatenateBlockTreeFixture, pointing_to_first_occurrence_check) {
    for (std::vector<Block*> level : block_tree_->levelwise_iterator()) {
        std::string level_string = "";
        std::vector<int> cumulated_length;
        for (int i = 0; i <level.size(); ++i) {
            Block* b = level[i];
            level_string += b->represented_string();
            cumulated_length.push_back((cumulated_length.size() == 0) ? b->length() : cumulated_length.back()+b->length());
            int index = level_string.find(b->represented_string());
            if (dynamic_cast<BackBlock*>(b)) EXPECT_LT(index, cumulated_length[b->first_occurrence_level_index_]);
        }
    }
}