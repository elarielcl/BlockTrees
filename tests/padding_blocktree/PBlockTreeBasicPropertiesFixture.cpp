//
// Created by sking32 on 3/11/18.
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

typedef PBlockTree* CreateBlockTreeFunc(int, int, std::string);



PBlockTree* other_conservative_heuristic_block_tree(int r, int max_leaf_length, std::string input) {
    PBlockTree* block_tree_ = new PBlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers_other_conservative_heuristic();
    return block_tree_;
}

PBlockTree* real_other_conservative_heuristic_block_tree(int r, int max_leaf_length, std::string input) {
    PBlockTree* block_tree_ = new PBlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers_real_other_conservative_heuristic();
    return block_tree_;
}

PBlockTree* real_conservative_heuristic_block_tree(int r, int max_leaf_length, std::string input) {
    PBlockTree* block_tree_ = new PBlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers_real_conservative_heuristic();
    return block_tree_;
}

PBlockTree* real_other_back_front_block_tree(int r, int max_leaf_length, std::string input) {
    PBlockTree* block_tree_ = new PBlockTree(input, r, max_leaf_length);
    block_tree_->process_real_other_back_front_pointers();
    return block_tree_;
}

PBlockTree* other_back_front_block_tree(int r, int max_leaf_length, std::string input) {

    PBlockTree* block_tree_ = new PBlockTree(input, r, max_leaf_length);
    block_tree_->process_other_back_front_pointers();
    return block_tree_;
}

PBlockTree* back_front_block_tree(int r, int max_leaf_length, std::string input) {

    PBlockTree* block_tree_ = new PBlockTree(input, r, max_leaf_length);
    block_tree_->process_back_front_pointers();
    return block_tree_;
}

PBlockTree* real_back_front_block_tree(int r, int max_leaf_length, std::string input) {
    PBlockTree* block_tree_ = new PBlockTree(input, r, max_leaf_length);
    block_tree_->process_real_back_front_pointers();
    return block_tree_;
}

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

PBlockTree* real_conservative_heuristic_3_block_tree(int r, int max_leaf_length, std::string input) {
    PBlockTree* block_tree_ = new PBlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers_real_conservative_heuristic(3);
    return block_tree_;
}

PBlockTree* conservative_heuristic_3_block_tree(int r, int max_leaf_length, std::string input) {
    PBlockTree* block_tree_ = new PBlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers_conservative_heuristic(3);
    return block_tree_;
}

PBlockTree* reverse_conservative_heuristic_3_block_tree(int r, int max_leaf_length, std::string input) {
    PBlockTree* block_tree_ = new PBlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers_reverse_conservative_heuristic(3);
    return block_tree_;
}


PBlockTree* block_tree_3_pruning(int r, int max_leaf_length, std::string input) {
    PBlockTree* block_tree_ = new PBlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers();
    block_tree_->clean_unnecessary_expansions(3);
    return block_tree_;
}

PBlockTree* real_conservative_optimized_heuristic_block_tree(int r, int max_leaf_length, std::string input) {
    PBlockTree* block_tree_ = new PBlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers_real_conservative_optimized_heuristic();
    return block_tree_;
}

PBlockTree* conservative_optimized_heuristic_block_tree(int r, int max_leaf_length, std::string input) {
    PBlockTree* block_tree_ = new PBlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers_conservative_optimized_heuristic();
    return block_tree_;
}

class PBlockTreeBasicPropertiesFixture : public ::testing::TestWithParam<::testing::tuple<int, int, std::string, CreateBlockTreeFunc*>> {
protected:
    virtual void TearDown() {
        delete block_tree_;
        delete block_tree_rs_;
    }

    virtual void SetUp() {
        CreateBlockTreeFunc* create_blocktree = ::testing::get<3>(GetParam());
        r_ = ::testing::get<0>(GetParam());
        leaf_length_ = ::testing::get<1>(GetParam());

        std::ifstream t(::testing::get<2>(GetParam()));
        std::stringstream buffer;
        buffer << t.rdbuf();
        input_= buffer.str();
        block_tree_ = (*create_blocktree)(r_ , leaf_length_, input_);


        block_tree_rs_ = (*create_blocktree)(r_ , leaf_length_, input_);

        std::unordered_set<int> characters;
        for (char c: input_)
            characters.insert(c);
        for (int c: characters) {
            characters_[c] = {};
            block_tree_rs_->add_rank_select_support(c);
        }
        block_tree_rs_->add_rank_select_leaf_support();
        block_tree_rs_->add_fwdsearch_support();
        block_tree_rs_->add_max_fields();
        block_tree_rs_->add_preffix_min_count_fields();
        for (int i = 0; i<input_.size(); ++i)
            characters_[input_[i]].push_back(i);
    }

public:
    PBlockTree* block_tree_;
    PBlockTree* block_tree_rs_;

    std::string input_;
    int r_;
    int leaf_length_;
    std::unordered_map<int,std::vector<int>> characters_; // Characters in the input and its select results

    PBlockTreeBasicPropertiesFixture() : ::testing::TestWithParam<::testing::tuple<int, int, std::string, CreateBlockTreeFunc*>>() {
    }

    virtual ~PBlockTreeBasicPropertiesFixture() {
    }
};

INSTANTIATE_TEST_CASE_P(AllVariantsTest,
                        PBlockTreeBasicPropertiesFixture,
                        Combine(Values(2),
                                Values(4),
                                Values(/*"../../tests/data/as", "../../tests/data/dna",*/ "../../tests/data/dna.par"/*, "../../tests/data/einstein"*/),
                                Values(&real_conservative_heuristic_3_block_tree,&real_conservative_optimized_heuristic_block_tree, &other_conservative_heuristic_block_tree, &real_other_conservative_heuristic_block_tree, &real_conservative_heuristic_block_tree/*,&real_other_back_front_block_tree,&other_back_front_block_tree,&back_front_block_tree,&real_back_front_block_tree,&block_tree, &block_tree_without_cleanning, &heuristic_concatenate_block_tree, &heuristic_block_tree,
                                       &liberal_heuristic_block_tree, &conservative_heuristic_block_tree, &conservative_heuristic_3_block_tree, &conservative_optimized_heuristic_block_tree,
                                       &reverse_conservative_heuristic_3_block_tree, &block_tree_3_pruning*/)));

// This test checks if the parameters given to the tree
// are the same inside the components of the BlockTree
TEST_P(PBlockTreeBasicPropertiesFixture, parameters_check) {
    EXPECT_EQ(block_tree_->r_, r_);
    EXPECT_EQ(block_tree_->input_, input_);
    EXPECT_EQ(block_tree_->leaf_length_, leaf_length_);
    std::vector< std::vector<PBlock*> > levels = block_tree_->levelwise_iterator();

    for (std::vector<PBlock*> level : levels)
        for (PBlock* b : level) {
            EXPECT_EQ(b->r_, r_);
            EXPECT_EQ(b->source_, input_);
            EXPECT_EQ(b->leaf_length_, leaf_length_);
        }
}

// This test checks if the internal nodes of the PBlockTree
// have r_ children except the last of each level
TEST_P(PBlockTreeBasicPropertiesFixture, almost_always_r_children_property_check) {
    for (std::vector<PBlock*> level : block_tree_->levelwise_iterator()) {
        for (int i = 0; i < level.size()-1; ++i) {
            PBlock* b =  level[i];
            if (dynamic_cast<PLazyInternalBlock*>(b) || dynamic_cast<PInternalBlock*>(b))
                EXPECT_EQ(r_, b->children_.size());
        }
        if (dynamic_cast<PLazyInternalBlock*>(level.back()) || dynamic_cast<PInternalBlock*>(level.back()))
            EXPECT_GE(r_, level.back()->children_.size());
    }
}


// This test checks if the internal and back blocks' lengths
// are > ll and if the leaves' lengths are = ll
TEST_P(PBlockTreeBasicPropertiesFixture, max_leaf_length_property_check) {
    for (std::vector<PBlock*> level : block_tree_->levelwise_iterator()) {
        for (PBlock* b : level) {
            if (dynamic_cast<PLeafBlock*>(b))
                EXPECT_EQ(b->length(), leaf_length_);
            else {
                EXPECT_GT(b->length(), leaf_length_);
            }
        }
    }
}

// This test checks if the BlockTree represents its input
// string and if the access method is correct
TEST_P(PBlockTreeBasicPropertiesFixture, input_integrity_or_access_check) {
    for (int i = 0; i < input_.size(); ++i)
        EXPECT_EQ(block_tree_->access(i), input_[i]);
}

// This test checks the rank method for every character
// and position in the input
TEST_P(PBlockTreeBasicPropertiesFixture, ranks_check) {
    for (auto pair : characters_) {
        int c = pair.first;
        int r = 0;
        for (int i = 0; i < input_.size(); ++i) {
            if (input_[i] == c) ++r;
            EXPECT_EQ(block_tree_rs_->rank(c, i), r);
        }
    }
}

// This test checks the rank_alternative method for every character
// and position in the input
TEST_P(PBlockTreeBasicPropertiesFixture, ranks_alternative_check) {
    for (auto pair : characters_) {
        int c  = pair.first;
        int r = 0;
        for (int i = 0; i < input_.size(); ++i) {
            if (input_[i] == c) ++r;
            EXPECT_EQ(block_tree_rs_->rank_alternative(c, i), r);
        }
    }
}


// This test checks the better_rank method for every character
// and position in the input
TEST_P(PBlockTreeBasicPropertiesFixture, better_ranks_check) {
    for (auto pair : characters_) {
        int c  = pair.first;
        int r = 0;
        for (int i = 0; i < input_.size(); ++i) {
            if (input_[i] == c) ++r;
            EXPECT_EQ(block_tree_rs_->better_rank(c, i), r);
        }
    }
}

// This test checks the select method for every character
// and rank
TEST_P(PBlockTreeBasicPropertiesFixture, selects_check) {
    for (auto pair : characters_) {
        int c  = pair.first;
        for (int j = 1; j<=pair.second.size(); ++j)
            EXPECT_EQ(block_tree_rs_->select(c, j), pair.second[j-1]);
    }
}


// This test checks the select_alternative method for every character
// and rank
TEST_P(PBlockTreeBasicPropertiesFixture, selects_alternative_check) {
    for (auto pair : characters_) {
        int c  = pair.first;
        for (int j = 1; j<=pair.second.size(); ++j)
            EXPECT_EQ(block_tree_rs_->select_alternative(c, j), pair.second[j-1]) << c << " " << j;
    }
}

// This test checks the better_select method for every character
// and rank
TEST_P(PBlockTreeBasicPropertiesFixture, better_selects_check) {
    for (auto pair : characters_) {
        int c  = pair.first;
        for (int j = 1; j<=pair.second.size(); ++j)
            EXPECT_EQ(block_tree_rs_->better_select(c, j), pair.second[j-1]) << c << " " << j;
    }
}

// This test checks the leaf_rank method for every
// position in the input
TEST_P(PBlockTreeBasicPropertiesFixture, leaf_ranks_check) {
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
                EXPECT_EQ(block_tree_rs_->leaf_rank(i), r);
        }
}

// This test checks the leaf_rank_alternative method for every
// position in the input
TEST_P(PBlockTreeBasicPropertiesFixture, leaf_ranks_alternative_check) {
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
        EXPECT_EQ(block_tree_rs_->leaf_rank_alternative(i), r);
    }
}

// This test checks the better_leaf_rank method for every
// position in the input
TEST_P(PBlockTreeBasicPropertiesFixture, better_leaf_ranks_check) {
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
        EXPECT_EQ(block_tree_rs_->better_leaf_rank(i), r);
    }
}


// This test checks the leaf_select method for every
// leaf in the input
TEST_P(PBlockTreeBasicPropertiesFixture, leaf_selects_check) {
    int r = 0;
    bool one_seen = false;
    for (int i = 0; i < input_.length(); ++i) {
        if (input_[i] == input_[0]) {
            one_seen = true;
        } else {
            if (one_seen) {
                ++r;
                EXPECT_EQ(block_tree_rs_->leaf_select(r), i-1);
            }
            one_seen = false;
        }
    }
}

// This test checks the leaf_select_alternative method for every
// leaf in the input
TEST_P(PBlockTreeBasicPropertiesFixture, leaf_selects_alternative_check) {
    int r = 0;
    bool one_seen = false;
    for (int i = 0; i < input_.length(); ++i) {
        if (input_[i] == input_[0]) {
            one_seen = true;
        } else {
            if (one_seen) {
                ++r;
                EXPECT_EQ(block_tree_rs_->leaf_select_alternative(r), i-1);
            }
            one_seen = false;
        }
    }
}

// This test checks the better_leaf_select method for every
// leaf in the input
TEST_P(PBlockTreeBasicPropertiesFixture, better_leaf_selects_check) {
    int r = 0;
    bool one_seen = false;
    for (int i = 0; i < input_.length(); ++i) {
        if (input_[i] == input_[0]) {
            one_seen = true;
        } else {
            if (one_seen) {
                ++r;
                EXPECT_EQ(block_tree_rs_->better_leaf_select(r), i-1);
            }
            one_seen = false;
        }
    }
}

// This test checks the fwdsearch method for every character
// in the input and d = {-1, -2} works correctly
TEST_P(PBlockTreeBasicPropertiesFixture, fwdsearch_check) {
    std::unordered_set<int> ds = {-1, -2};
    for (int d : ds) {
        for (int i = 0; i < input_.length(); ++i) {
            int search = block_tree_rs_->fwdsearch(i, d);
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
TEST_P(PBlockTreeBasicPropertiesFixture, bwdsearch_check) {
    std::unordered_set<int> ds = {-1, -2};
    for (int d : ds) {
        for (int i = 0; i < input_.length(); ++i) {
            int search = block_tree_rs_->bwdsearch(i, d);
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
TEST_P(PBlockTreeBasicPropertiesFixture, positive_fwdsearch_check) {
    std::unordered_set<int> ds = {1, 2};
    for (int d : ds) {
        for (int i = 0; i < input_.length(); ++i) {
            int search = block_tree_rs_->positive_fwdsearch(i, d);
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
TEST_P(PBlockTreeBasicPropertiesFixture, positive_bwdsearch_check) {
    std::unordered_set<int> ds = {1, 2};
    for (int d : ds) {
        for (int i = 0; i < input_.length(); ++i) {
            int search = block_tree_rs_->positive_bwdsearch(i, d);
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


// This test checks the min_excess(i,j) method for some sample
TEST_P(PBlockTreeBasicPropertiesFixture, min_excess_check) {
    for (int j = 0; j < input_.length(); j = 2*j+1) {
        for (int i = 0; i <= j; i = 2*i+1) {
            EXPECT_EQ(block_tree_rs_->root_block_->linear_min_excess(i,j), block_tree_rs_->min_excess(i,j));
        }
    }
}

// This test checks the min_count(i,j) method for some sample
TEST_P(PBlockTreeBasicPropertiesFixture, min_count_check) {
    for (int j = 0; j < input_.length(); j = 2*j+1) {
        for (int i = 0; i <= j; i = 2*i+1) {
            EXPECT_EQ(block_tree_rs_->root_block_->linear_min_count(i,j), block_tree_rs_->min_count(i,j));
        }
    }
}


// This test checks the min_select(i,j,t) method for some sample
TEST_P(PBlockTreeBasicPropertiesFixture, min_select_check) {
    for (int j = 0; j < input_.length(); j = 2*j+1) {
        for (int i = 0; i <= j; i = 2*i+1) {
            int count = block_tree_rs_->min_count(i,j);
            for (int t = 1; t <= count ; ++t) {
                EXPECT_EQ(block_tree_rs_->root_block_->linear_min_select(i,j,t), block_tree_rs_->min_select(i,j,t));
            }
        }
    }
}


// This test checks the max_excess(i,j) method for some sample
TEST_P(PBlockTreeBasicPropertiesFixture, max_excess_check) {
    for (int j = 0; j < input_.length(); j = 2*j+1) {
        for (int i = 0; i <= j; i = 2*i+1) {
            EXPECT_EQ(block_tree_rs_->root_block_->linear_max_excess(i,j), block_tree_rs_->max_excess(i,j));
        }
    }
}

// This test checks the property that all the leaves are in the
// last level of the PBlockTree
TEST_P(PBlockTreeBasicPropertiesFixture, all_leaves_last_level_property_check) {
    auto iterator = block_tree_->levelwise_iterator();
    for (int i = 0; i < iterator.size()-1; ++i) {
        std::vector<PBlock*> level = iterator[i];
        for (PBlock* b : level)
            EXPECT_FALSE(dynamic_cast<PLeafBlock*>(b));
    }
    for (PBlock* b : iterator.back())
        EXPECT_TRUE(dynamic_cast<PLeafBlock*>(b));
}


// This test checks that in a level all the blocks has the same length
TEST_P(PBlockTreeBasicPropertiesFixture, same_level_length_property_check) {
    std::vector< std::vector<PBlock*> > levels = block_tree_->levelwise_iterator();

    for (std::vector<PBlock*> level : levels) {
        int l = level.front()->length();
        for (PBlock* b : level)
            EXPECT_TRUE(l == b->length());
    }
}


// This test checks if the indices are assigned correctly
TEST_P(PBlockTreeBasicPropertiesFixture, correct_children_indices_property_check) {
    int padded_length = 1;
    while (padded_length < input_.size()) padded_length *= r_;

    EXPECT_EQ(block_tree_->root_block_->start_index_, 0);
    EXPECT_EQ(block_tree_->root_block_->end_index_, padded_length-1);

    for (std::vector<PBlock*> level : block_tree_->levelwise_iterator()) {
        for (int i = 0; i < level.size()-1; ++i) {
            PBlock* b = level[i];
            if (dynamic_cast<PLazyInternalBlock*>(b) || dynamic_cast<PInternalBlock*>(b)) {
                EXPECT_EQ(b->start_index_, b->children_.front()->start_index_);
                EXPECT_EQ(b->end_index_, b->children_.back()->end_index_);

                int l = b->children_[0]->length();
                int last_end = b->start_index_-1;
                for (PBlock* child : b->children_) {
                    EXPECT_EQ(l, child->length());
                    EXPECT_EQ(last_end+1, child->start_index_);
                    last_end = child->end_index_;
                }

            }
        }
        PBlock* b = level.back();
        if (dynamic_cast<PLazyInternalBlock*>(b) || dynamic_cast<PInternalBlock*>(b)) {
            EXPECT_EQ(b->start_index_, b->children_.front()->start_index_);
            EXPECT_GE(b->end_index_, b->children_.back()->end_index_);

            int l = b->children_[0]->length();
            int last_end = b->start_index_-1;
            for (PBlock* child : b->children_) {
                EXPECT_EQ(l, child->length());
                EXPECT_EQ(last_end+1, child->start_index_);
                last_end = child->end_index_;
            }

        }


    }
}

// This test checks if the fields parent and child_number
// are correct
TEST_P(PBlockTreeBasicPropertiesFixture, parent_and_child_number_check) {
    for (std::vector<PBlock*> level : block_tree_->levelwise_iterator()) {
        for (PBlock* b: level) {
            if (dynamic_cast<PLazyInternalBlock*>(b) || dynamic_cast<PInternalBlock*>(b)) {
                for (int i = 0; i < b->children_.size(); ++i) {
                    PBlock* child = b->children_[i];
                    EXPECT_EQ(child->parent_, b);
                    EXPECT_EQ(child->child_number_, i);
                }
            }
        }
    }
}


// This test checks if the data_ in the LeafBlock is correct
TEST_P(PBlockTreeBasicPropertiesFixture, leaf_block_representation_check) {
    for (std::vector<PBlock*> level : block_tree_->levelwise_iterator()) {
        for (PBlock* b: level) {
            if (PLeafBlock* l = dynamic_cast<PLeafBlock*>(b)) {
                EXPECT_EQ(l->data_, input_.substr(l->start_index_, l->length()));
            }
        }
    }
}

// This test checks if the pointed string (by a back pointer)
// is correct
TEST_P(PBlockTreeBasicPropertiesFixture, back_pointer_right_representation_check) {
    for (std::vector<PBlock*> level : block_tree_->levelwise_iterator()) {
        for (PBlock* b: level) {
            if (dynamic_cast<PBackBlock*>(b)) {
                std::string data = b->first_block_->represented_string().substr(b->offset_, b->length());
                if (b->second_block_ != nullptr)
                    data += b->second_block_->represented_string().substr(0, b->length()-data.length());
                EXPECT_EQ(b->represented_string(), data);
            }
        }
    }
}



// This test checks if the first pointed block (by a back pointer)
// occurrs before on its level
TEST_P(PBlockTreeBasicPropertiesFixture, back_pointer_points_back_check) {
    for (std::vector<PBlock*> level : block_tree_->levelwise_iterator()) {
        int max_level_length = level.front()->length();
        for (PBlock* b: level) {
            if (dynamic_cast<PBackBlock*>(b)) {
                EXPECT_LT(b->first_block_->end_index_, b->start_index_);
                EXPECT_TRUE(b->first_block_->length() == max_level_length || b->first_block_->length() == max_level_length-1);
            }
        }
    }
}

// This test checks if the block pointed by a back block are consecutive
// Also, it checks that the fields level_index and first_occurrence_level_index
// are right
TEST_P(PBlockTreeBasicPropertiesFixture, consecutive_blocks_back_pointer_and_indices_fields_check) {
    for (std::vector<PBlock*> level : block_tree_->levelwise_iterator()) {
        for (int i = 0; i < level.size(); ++i) {
            PBlock* b = level[i];
            EXPECT_EQ(i, b->level_index_);
            EXPECT_LE(b->first_occurrence_level_index_, b->level_index_);
            if (dynamic_cast<PBackBlock*>(b)) {
                EXPECT_EQ(b->first_block_, level[b->first_occurrence_level_index_]);
                if (b->second_block_ != nullptr) EXPECT_EQ(b->second_block_, level[b->first_occurrence_level_index_+1]);
            }

        }
    }
}

// This test checks if the pointed string (by a back pointer)
// fits on the pointed blocks
TEST_P(PBlockTreeBasicPropertiesFixture, back_pointer_fit_check) {
    for (std::vector<PBlock*> level : block_tree_->levelwise_iterator()) {
        for (PBlock* b: level) {
            if (dynamic_cast<PBackBlock*>(b)) {
                if (b->second_block_ == nullptr) {
                    EXPECT_TRUE(!b->offset_);
                } else {
                    EXPECT_LT(b->offset_+b->length()-1-b->first_block_->length(), b->second_block_->length());
                }
            }
        }
    }
}



// This test checks if the prefix_ranks field is correct
TEST_P(PBlockTreeBasicPropertiesFixture, preffix_ranks_field_check) {
    for (std::vector<PBlock*> level : block_tree_rs_->levelwise_iterator()) {
        for (PBlock* b: level) {
            if (dynamic_cast<PLazyInternalBlock*>(b) || dynamic_cast<PInternalBlock*>(b)) {
                std::unordered_map<int,int> prefix_ranks;
                for (auto pair : b->prefix_ranks_) {
                    prefix_ranks[pair.first] = 0;
                }
                for (PBlock* child : b->children_) {
                    for (auto pair: prefix_ranks)
                        EXPECT_EQ(prefix_ranks[pair.first], child->prefix_ranks_[pair.first]);
                    for (int i = child->start_index_; i<= child->end_index_; ++i)
                        prefix_ranks[input_[i]] = prefix_ranks[input_[i]] + 1;
                }

            }
        }
    }
}



// This test checks if the ranks field is correct
TEST_P(PBlockTreeBasicPropertiesFixture, ranks_field_check) {
    for (std::vector<PBlock*> level : block_tree_rs_->levelwise_iterator()) {
        for (PBlock *b: level) {
            std::unordered_map<int, int> ranks;
            for (auto pair: b->ranks_)
                ranks[pair.first] = 0;
            for (int i = b->start_index_; i <= b->end_index_ && i < input_.size(); ++i)
                ranks[input_[i]] = ranks[input_[i]] + 1;
            for (auto pair : ranks)
                EXPECT_EQ(ranks[pair.first], b->ranks_[pair.first]);
        }
    }
}

// This test checks if the cumulated_ranks field is correct
TEST_P(PBlockTreeBasicPropertiesFixture, cumulated_ranks_field_check) {
    for (std::vector<PBlock*> level : block_tree_rs_->levelwise_iterator()) {
        for (PBlock* b: level) {
            if (dynamic_cast<PLazyInternalBlock*>(b) || dynamic_cast<PInternalBlock*>(b)) {
                std::unordered_map<int,int> cumulated_ranks;
                for (auto pair : b->cumulated_ranks_) {
                    cumulated_ranks[pair.first] = 0;
                }
                for (PBlock* child : b->children_) {
                    for (int i = child->start_index_; i<= child->end_index_ && i < input_.size(); ++i)
                        cumulated_ranks[input_[i]] = cumulated_ranks[input_[i]] + 1;
                    for (auto pair: cumulated_ranks)
                        EXPECT_EQ(cumulated_ranks[pair.first], child->cumulated_ranks_[pair.first]);

                }

            }
        }
    }
}


// This test checks if the first and second_ranks field are correct
TEST_P(PBlockTreeBasicPropertiesFixture, first_second_ranks_field_check) {
    for (std::vector<PBlock*> level : block_tree_rs_->levelwise_iterator()) {
        for (PBlock* b: level) {
            if (dynamic_cast<PBackBlock*>(b)) {
                std::unordered_map<int,int> first_ranks;
                std::unordered_map<int,int> second_ranks;
                for (auto pair: b->first_ranks_)
                    first_ranks[pair.first] = 0;
                for (auto pair: b->second_ranks_)
                    second_ranks[pair.second] = 0;
                int i = b->first_block_->start_index_;
                for (; i < b->first_block_->start_index_ +  b->offset_; ++i)
                    first_ranks[input_[i]] = first_ranks[input_[i]] + 1;
                for (; i <= b->first_block_->end_index_ && i < b->first_block_->start_index_ + b->offset_ + b->length(); ++i)
                    second_ranks[input_[i]] = second_ranks[input_[i]] + 1;

                for (auto pair: first_ranks)
                    EXPECT_EQ(first_ranks[pair.first], b->first_ranks_[pair.first]);
                for (auto pair: second_ranks)
                    EXPECT_EQ(second_ranks[pair.first], b->second_ranks_[pair.first]);
            }
        }
    }
}

// This test checks if the prefix_leaf_rank field is correct
TEST_P(PBlockTreeBasicPropertiesFixture, preffix_leaf_ranks_field_check) {
    for (std::vector<PBlock*> level : block_tree_rs_->levelwise_iterator()) {
        for (PBlock* b: level) {
            if (dynamic_cast<PLazyInternalBlock*>(b) || dynamic_cast<PInternalBlock*>(b)) {
                int prefix_leaf_rank = 0;
                for (PBlock* child : b->children_) {
                    EXPECT_EQ(prefix_leaf_rank, child->prefix_leaf_rank_);
                    int r = 0;
                    bool one_seen = child->starts_with_end_leaf_;
                    for (int j = child->start_index_; j<=child->end_index_; ++j) {
                        if (input_[j] == input_[0]) {
                            one_seen = true;
                        } else {
                            if (one_seen) {
                                ++r;
                            }
                            one_seen = false;
                        }
                    }
                    prefix_leaf_rank += r;
                }

            }
        }
    }
}

// This test checks if the leaf_rank field is correct
TEST_P(PBlockTreeBasicPropertiesFixture, leaf_rank_field_check) {
    for (std::vector<PBlock*> level : block_tree_rs_->levelwise_iterator()) {
        for (PBlock *b: level) {
            int leaf_rank = 0;
            int r = 0;
            bool one_seen = b->starts_with_end_leaf_;
            for (int j = b->start_index_; j<=b->end_index_&& j < input_.size(); ++j) {
                if (input_[j] == input_[0]) {
                    one_seen = true;
                } else {
                    if (one_seen) {
                        ++r;
                    }
                    one_seen = false;
                }
            }
            leaf_rank = r;
            EXPECT_EQ(leaf_rank, b->leaf_rank_);
        }
    }
}

// This test checks if the cumulated_leaf_rank field is correct
TEST_P(PBlockTreeBasicPropertiesFixture, cumulated_leaf_ranks_field_check) {
    for (std::vector<PBlock*> level : block_tree_rs_->levelwise_iterator()) {
        for (PBlock* b: level) {
            if (dynamic_cast<PLazyInternalBlock*>(b) || dynamic_cast<PInternalBlock*>(b)) {
                int cumulated_leaf_rank = 0;
                for (PBlock* child : b->children_) {
                    int r = 0;
                    bool one_seen = child->starts_with_end_leaf_;
                    for (int j = child->start_index_; j<=child->end_index_ && j < input_.size(); ++j) {
                        if (input_[j] == input_[0]) {
                            one_seen = true;
                        } else {
                            if (one_seen) {
                                ++r;
                            }
                            one_seen = false;
                        }
                    }
                    cumulated_leaf_rank += r;
                    EXPECT_EQ(cumulated_leaf_rank, child->cumulated_leaf_rank_);
                }

            }
        }
    }
}


// This test checks if the first and second_leaf_ranks fields are correct
TEST_P(PBlockTreeBasicPropertiesFixture, first_second_leaf_rank_field_check) {
    for (std::vector<PBlock*> level : block_tree_rs_->levelwise_iterator()) {
        for (PBlock* b: level) {
            if (dynamic_cast<PBackBlock*>(b)) {
                int first_rank_leaf = 0;
                int second_rank_leaf = 0;

                int r = 0;
                bool one_seen = b->first_block_->starts_with_end_leaf_;
                int j = b->first_block_->start_index_;
                for (; j < b->first_block_->start_index_ +  b->offset_; ++j) {
                    if (input_[j] == input_[0]) {
                        one_seen = true;
                    } else {
                        if (one_seen) {
                            ++r;
                        }
                        one_seen = false;
                    }
                }
                first_rank_leaf = r;
                r = 0;
                for (; j <= b->first_block_->end_index_ && j < b->first_block_->start_index_ + b->offset_ + b->length(); ++j) {
                    if (input_[j] == input_[0]) {
                        one_seen = true;
                    } else {
                        if (one_seen) {
                            ++r;
                        }
                        one_seen = false;
                    }
                }
                second_rank_leaf = r;

                EXPECT_EQ(first_rank_leaf, b->first_leaf_rank_);
                EXPECT_EQ(second_rank_leaf, b->second_leaf_rank_);
            }
        }
    }
}


// This test checks if the leaf_end_bits are correct
TEST_P(PBlockTreeBasicPropertiesFixture, leaf_end_bits_field_check) {
    for (std::vector<PBlock *> level : block_tree_rs_->levelwise_iterator()) {
        for (PBlock *b: level) {
            EXPECT_EQ(b->starts_with_end_leaf_, input_[b->start_index_] != input_[0] && input_[b->start_index_-1] == input_[0]);
            if (dynamic_cast<PBackBlock *>(b)) {
                PBackBlock * bb = dynamic_cast<PBackBlock*>(b);
                EXPECT_EQ(bb->suffix_start_with_end_leaf_, input_[bb->first_block_->start_index_+bb->offset_] != input_[0] && input_[bb->first_block_->start_index_+bb->offset_-1] == input_[0]);
                EXPECT_EQ(bb->prefix_start_with_end_leaf_, input_[bb->start_index_+bb->first_block_->length()-bb->offset_] != input_[0] && input_[bb->start_index_+bb->first_block_->length()-bb->offset_-1] == input_[0]);
            }
        }
    }
}


// This test checks if the min_prefix_excess_,
// prefix_first_block_min_excess_, prefix_second_block_min_excess_
// are correct
TEST_P(PBlockTreeBasicPropertiesFixture, prefix_min_excess_field_check) {
    for (std::vector<PBlock *> level : block_tree_rs_->levelwise_iterator()) {
        for (PBlock *b: level) {
            EXPECT_EQ(b->min_prefix_excess_, b->min_excess());
            if (dynamic_cast<PBackBlock *>(b)) {
                PBackBlock * bb = dynamic_cast<PBackBlock*>(b);
                int excess = 0;
                int min_excess = 1;
                for (int i = 0; i < bb->first_block_->length() - bb->offset_; ++i) {
                    excess += (input_[b->start_index_+i] == input_[0]) ? 1:-1;
                    if (excess < min_excess) min_excess = excess;
                }
                EXPECT_EQ(min_excess, bb->prefix_first_block_min_excess_);
                if (bb->second_block_ == nullptr)
                    EXPECT_EQ(bb->prefix_second_block_min_excess_, 1);
                else {
                    int excess = 0;
                    int min_excess = 1;
                    for (int i = bb->first_block_->length() - bb->offset_; i < bb->length(); ++i) {
                        excess += (input_[b->start_index_+i] == input_[0]) ? 1:-1;
                        if (excess < min_excess) min_excess = excess;
                    }
                    EXPECT_EQ(min_excess, bb->prefix_second_block_min_excess_);
                }
            }
        }
    }
}


// This test checks if the max_prefix_excess_,
// prefix_first_block_max_excess_, prefix_second_block_max_excess_
// are correct
TEST_P(PBlockTreeBasicPropertiesFixture, prefix_max_excess_field_check) {
    for (std::vector<PBlock *> level : block_tree_rs_->levelwise_iterator()) {
        for (PBlock *b: level) {
            EXPECT_EQ(b->max_prefix_excess_, b->max_excess());
            if (dynamic_cast<PBackBlock *>(b)) {
                PBackBlock * bb = dynamic_cast<PBackBlock*>(b);
                int excess = 0;
                int max_excess = -1;
                for (int i = 0; i < bb->first_block_->length() - bb->offset_; ++i) {
                    excess += (input_[b->start_index_+i] == input_[0]) ? 1:-1;
                    if (excess > max_excess) max_excess = excess;
                }
                EXPECT_EQ(max_excess, bb->prefix_first_block_max_excess_);
                if (bb->second_block_ == nullptr)
                    EXPECT_EQ(bb->prefix_second_block_max_excess_, -1);
                else {
                    int excess = 0;
                    int max_excess = -1;
                    for (int i = bb->first_block_->length() - bb->offset_; i < bb->length(); ++i) {
                        excess += (input_[b->start_index_+i] == input_[0]) ? 1:-1;
                        if (excess > max_excess) max_excess = excess;
                    }
                    EXPECT_EQ(max_excess, bb->prefix_second_block_max_excess_);
                }
            }
        }
    }
}

// This test checks if the prefix_min_count_,
// prefix_min_first_block_count_, prefix_min_second_block_count_
// are correct
TEST_P(PBlockTreeBasicPropertiesFixture, prefix_min_count_field_check) {
    for (std::vector<PBlock *> level : block_tree_rs_->levelwise_iterator()) {
        for (PBlock *b: level) {
            EXPECT_EQ(b->prefix_min_count_, b->min_count());
            if (dynamic_cast<PBackBlock *>(b)) {
                PBackBlock * bb = dynamic_cast<PBackBlock*>(b);
                int excess = 0;
                int min_excess = 1;
                int count = 0;
                for (int i = 0; i < bb->first_block_->length() - bb->offset_; ++i) {
                    excess += (input_[b->start_index_+i] == input_[0]) ? 1:-1;
                    if (excess == min_excess) ++count;
                    if (excess < min_excess) {
                        min_excess = excess;
                        count = 1;
                    }
                }
                EXPECT_EQ(count, bb->prefix_min_first_block_count_);
                if (bb->second_block_ == nullptr)
                    EXPECT_EQ(bb->prefix_min_second_block_count_, 0);
                else {
                    int excess = 0;
                    int min_excess = 1;
                    int count = 0;
                    for (int i = bb->first_block_->length() - bb->offset_; i < bb->length(); ++i) {
                        excess += (input_[b->start_index_+i] == input_[0]) ? 1:-1;
                        if (excess == min_excess) ++count;
                        if (excess < min_excess) {
                            min_excess = excess;
                            count = 1;
                        }
                    }
                    EXPECT_EQ(count, bb->prefix_min_second_block_count_);
                }
            }
        }
    }
}