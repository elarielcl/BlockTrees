
#include <bits/stdc++.h>
#include <padding_blocktree/PLeafBlock.h>
#include <padding_blocktree/PLazyInternalBlock.h>
#include <padding_blocktree/PInternalBlock.h>
#include <unordered_set>
#include <fstream>
#include <compressed/PBTCST.h>
#include "gtest/gtest.h"

#include "padding_blocktree/PBlockTree.h"

using ::testing::Combine;
using ::testing::Values;


class PBTCSTFixture : public ::testing::TestWithParam<std::string> { // Some tests for this class encoded for an specific input banananananana. Also, this doesn't check topology specifics methods
protected:
    virtual void TearDown() {
        delete cst_;
    }

    virtual void SetUp() {

        std::ifstream t(GetParam());
        std::stringstream buffer;
        buffer << t.rdbuf();
        input_= buffer.str();
        cst_ = new PBTCST(input_);

    }

public:


    std::string input_;
    PBTCST* cst_;

    PBTCSTFixture() : ::testing::TestWithParam<std::string>() {
    }

    virtual ~PBTCSTFixture() {
    }
};

INSTANTIATE_TEST_CASE_P(PBTCSTTest,
                        PBTCSTFixture,
                        Values("../../tests/data/banananananana"/*, "../../tests/data/einstein"*/));



// This test checks the string_depth of all the nodes
TEST_P(PBTCSTFixture, string_depth_check) {
    std::vector<int> string_depths = {0,1,1,2,3,4,5,6,7,8,9,10,11,12,14,15,2,3,4,5,6,7,8,9,10,11,13};
    for (int i = 0; i < string_depths.size(); ++i) {
        EXPECT_EQ(string_depths[i], cst_->string_depth(cst_->pbtcTree_->node_select(i+1)));
    }
}

// This test checks the labels of all the nodes
TEST_P(PBTCSTFixture, labels_check) {
    std::vector<int> string_depths = {0,1,1,2,3,4,5,6,7,8,9,10,11,12,14,15,2,3,4,5,6,7,8,9,10,11,13};
    std::vector<std::string> labels = {"",
                                       "$",
                                       "a",
                                       "a$",
                                       "ana",
                                       "ana$",
                                       "anana",
                                       "anana$",
                                       "ananana",
                                       "ananana$",
                                       "anananana",
                                       "anananana$",
                                       "ananananana",
                                       "ananananana$",
                                       "anananananana$",
                                       "banananananana$",
                                       "na",
                                       "na$",
                                       "nana",
                                       "nana$",
                                       "nanana",
                                       "nanana$",
                                       "nananana",
                                       "nananana$",
                                       "nanananana",
                                       "nanananana$",
                                       "nananananana$"};
    for (int i = 0; i < labels.size(); ++i) {
        int node = cst_->pbtcTree_->node_select(i+1);
        EXPECT_EQ(labels[i], cst_->string(node, 0, string_depths[i]-1));
    }
}


// This test checks the labeled_child for all the nodes
TEST_P(PBTCSTFixture, labeled_child_check) {
    std::vector<std::vector<std::pair<int,int>>> answers = {
                                                            {{'$',2},{'a',3},{'b',16},{'n',17}},
                                                            {},
                                                            {{'$',4},{'n',5}},
                                                            {},
                                                            {{'$',6},{'n',7}},
                                                            {},
                                                            {{'$',8},{'n',9}},
                                                            {},
                                                            {{'$',10},{'n',11}},
                                                            {},
                                                            {{'$',12},{'n',13}},
                                                            {},
                                                            {{'$',14},{'n',15}},
                                                            {},
                                                            {},
                                                            {},
                                                            {{'$',18},{'n',19}},
                                                            {},
                                                            {{'$',20},{'n',21}},
                                                            {},
                                                            {{'$',22},{'n',23}},
                                                            {},
                                                            {{'$',24},{'n',25}},
                                                            {},
                                                            {{'$',26},{'n',27}},
                                                            {},
                                                            {}
                                                            };

    for (int i = 0; i < answers.size(); ++i) {
        int node = cst_->pbtcTree_->node_select(i+1);
        for (auto pair : answers[i]) {
            EXPECT_EQ(cst_->pbtcTree_->node_select(pair.second), cst_->labeled_child(node, pair.first));
        }
    }
}

// This test suffix_link on every node
TEST_P(PBTCSTFixture, suffix_link_check) {
    std::vector<int> leaves_sequence = {16, 15, 27, 14, 26, 12, 24, 10, 22, 8, 20, 6, 18, 4, 2, 1};
    for (int i = 0; i < leaves_sequence.size()-1; ++i) {
        EXPECT_EQ(cst_->pbtcTree_->node_select(leaves_sequence[i+1]), cst_->suffix_link(cst_->pbtcTree_->node_select(leaves_sequence[i])));
    }


    std::vector<int> nodes_sequence = {13, 25, 11, 23, 9, 21, 7, 19, 5, 17, 3, 1, 1};
    for (int i = 0; i < nodes_sequence.size()-1; ++i) {
        EXPECT_EQ(cst_->pbtcTree_->node_select(nodes_sequence[i+1]), cst_->suffix_link(cst_->pbtcTree_->node_select(nodes_sequence[i])));
    }

    for (int i = 0; i < leaves_sequence.size()-1; ++i) {
        EXPECT_EQ(cst_->pbtcTree_->node_select(leaves_sequence[i+1]), cst_->suffix_link(cst_->pbtcTree_->node_select(16), i+1));
    }

    for (int i = 0; i < nodes_sequence.size()-1; ++i) {
        EXPECT_EQ(cst_->pbtcTree_->node_select(nodes_sequence[i+1]), cst_->suffix_link(cst_->pbtcTree_->node_select(13), i+1));
    }

}


// This test weiner link on every_node for its correspondent suffix link, then a sampling, then nonexistent links
TEST_P(PBTCSTFixture, weiner_link_check) {
    std::vector<std::pair<int,int>> leaves_sequence = {{'h',16}, {'b', 15}, {'a', 27}, {'n', 14}, {'a', 26}, {'n', 12}, {'a', 24}, {'n', 10}, {'a', 22}, {'n', 8}, {'a', 20}, {'n', 6}, {'a', 18}, {'n', 4}, {'a',2}, {'$', 1}};
    for (int i = leaves_sequence.size()-1; i > 0; --i) {
        EXPECT_EQ(cst_->pbtcTree_->node_select(leaves_sequence[i-1].second), cst_->weiner_link(cst_->pbtcTree_->node_select(leaves_sequence[i].second), leaves_sequence[i].first));
    }

    std::vector<std::pair<int,int>> nodes_sequence = {{'h', 13}, {'a', 25}, {'n', 11}, {'a', 23}, {'n', 9}, {'a', 21}, {'n', 7}, {'a', 19}, {'n', 5}, {'a', 17}, {'n',3}, {'a', 1}};
    for (int i = nodes_sequence.size()-1; i > 0; --i) {
        EXPECT_EQ(cst_->pbtcTree_->node_select(nodes_sequence[i-1].second), cst_->weiner_link(cst_->pbtcTree_->node_select(nodes_sequence[i].second), nodes_sequence[i].first));
    }


    EXPECT_EQ(cst_->pbtcTree_->node_select(16),
            cst_->weiner_link(cst_->pbtcTree_->node_select(15), 'b'));

    EXPECT_EQ(0,
            cst_->weiner_link(cst_->pbtcTree_->node_select(9), 'a'));
}



//This method check the locus method with some samples
TEST_P(PBTCSTFixture, locus_check) {
    EXPECT_EQ(cst_->pbtcTree_->node_select(5),
              cst_->locus("ana"));
    EXPECT_EQ(cst_->pbtcTree_->node_select(16),
              cst_->locus("b"));
    EXPECT_EQ(cst_->pbtcTree_->node_select(19),
              cst_->locus("nana"));
    EXPECT_EQ(-1,
              cst_->locus("bna"));
}


//This method check the string_ancestor method with some samples
TEST_P(PBTCSTFixture, string_ancestor_check) {
    EXPECT_EQ(cst_->pbtcTree_->node_select(5),
              cst_->string_ancestor(cst_->pbtcTree_->node_select(13), 3));

    EXPECT_EQ(cst_->pbtcTree_->node_select(19),
              cst_->string_ancestor(cst_->pbtcTree_->node_select(24), 4));


    EXPECT_EQ(cst_->pbtcTree_->node_select(16),
              cst_->string_ancestor(cst_->pbtcTree_->node_select(16), 4));
}