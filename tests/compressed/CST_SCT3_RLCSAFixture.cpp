//
// Created by elarielcl on 01/10/18.
//


//#include <bits/stdc++.h>
#include <padding_blocktree/PLeafBlock.h>
#include <padding_blocktree/PLazyInternalBlock.h>
#include <padding_blocktree/PInternalBlock.h>
#include <unordered_set>
#include <fstream>
#include <compressed/CST_SCT3_RLCSA.h>
#include <cst.jouni/cst_rep/includes/CST.h>
#include "gtest/gtest.h"

#include "padding_blocktree/PBlockTree.h"

using ::testing::Combine;
using ::testing::Values;


class CST_SCT3_RLCSAFixture : public ::testing::TestWithParam<std::string> { // Some tests for this class encoded for an specific input banana. Also, this doesn't check topology specifics methods
protected:
    virtual void TearDown() {
        delete cst_;
    }

    virtual void SetUp() {
        std::ifstream t(GetParam());
        std::stringstream buffer;
        buffer << t.rdbuf();
        input_= buffer.str();
        sdsl::cst_sct3<> cst;
        sdsl::construct_im(cst, input_, 1);
        for (auto& node : cst) {
            if (std::find(nodes.begin(), nodes.end(), node) == nodes.end())
                nodes.push_back(node);
        }

        cst_ = new CST_SCT3_RLCSA(input_);



        //cst = new CST("/home/elarielcl/Thesis/Software/BlockTree/cmake-build-debug/in", "/home/elarielcl/Thesis/Software/BlockTree/cmake-build-debug/albertotest", trlcsa, 10, 0);

    }

public:

    std::string input_;
    CST_SCT3_RLCSA* cst_;
    std::vector<sdsl::bp_interval<uint64_t>> nodes;
    //CST* cst;

    CST_SCT3_RLCSAFixture() : ::testing::TestWithParam<std::string>() {
    }

    virtual ~CST_SCT3_RLCSAFixture() {
    }
};

INSTANTIATE_TEST_CASE_P(PBTCSTTest,
        CST_SCT3_RLCSAFixture,
        Values("../../tests/data/banana"/*, "../../tests/data/einstein"*/));



// This test checks the string_depth of all the nodes
TEST_P(CST_SCT3_RLCSAFixture, string_depth_check) {
std::vector<int> string_depths = {0,1,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,22,23,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,21};
for (int i = 0; i < string_depths.size(); ++i) {
int f = string_depths[i];
int s = cst_->string_depth(nodes[i]);
if (f != s) {
cst_->string_depth(nodes[i]);
}
EXPECT_EQ(string_depths[i], cst_->string_depth(nodes[i]));
}
}

// This test checks the labels of all the nodes
TEST_P(CST_SCT3_RLCSAFixture, labels_check) {
std::vector<int> string_depths = {0,1,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,22,23,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,21};
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
                                   "anananananana",
                                   "anananananana$",
                                   "ananananananana",
                                   "ananananananana$",
                                   "anananananananana",
                                   "anananananananana$",
                                   "ananananananananana",
                                   "ananananananananana$",
                                   "anananananananananana$",
                                   "banananananananananana$",
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
                                   "nananananana",
                                   "nananananana$",
                                   "nanananananana",
                                   "nanananananana$",
                                   "nananananananana",
                                   "nananananananana$",
                                   "nanananananananana",
                                   "nanananananananana$",
                                   "nananananananananana$"};
for (int i = 0; i < labels.size(); ++i) {
auto node = nodes[i];
EXPECT_EQ(labels[i], cst_->string(node, 0, string_depths[i]-1));
for (int j = 0; j < string_depths[i]-1; ++j) {
EXPECT_EQ(labels[i][j], cst_->string(node, j));
}
}
}


// This test checks the labeled_child for all the nodes
TEST_P(CST_SCT3_RLCSAFixture, labeled_child_check) {
std::vector<std::vector<std::pair<int,int>>> answers = {
        {{'$',2},{'a',3},{'b',24},{'n',25}},
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
        {{'$',16},{'n',17}},
        {},
        {{'$',18},{'n',19}},
        {},
        {{'$',20},{'n',21}},
        {},
        {{'$',22},{'n',23}},
        {},
        {},
        {},
        {{'$',26},{'n',27}},
        {},
        {{'$',28},{'n',29}},
        {},
        {{'$',30},{'n',31}},
        {},
        {{'$',32},{'n',33}},
        {},
        {{'$',34},{'n',35}},
        {},
        {{'$',36},{'n',37}},
        {},
        {{'$',38},{'n',39}},
        {},
        {{'$',40},{'n',41}},
        {},
        {{'$',42},{'n',43}},
        {},
        {}
};

for (int i = 0; i < answers.size(); ++i) {
auto node = nodes[i];
for (auto pair : answers[i]) {
EXPECT_EQ(nodes[(pair.second)-1], cst_->labeled_child(node, pair.first));
}
}
}

// This test suffix_link on every node
TEST_P(CST_SCT3_RLCSAFixture, suffix_link_check) {
std::vector<int> leaves_sequence = {24, 23, 43, 22, 42, 20, 40, 18, 38, 16, 36, 14, 34, 12, 32, 10, 30, 8, 28, 6, 26, 4, 2, 1};
for (int i = 0; i < leaves_sequence.size()-1; ++i) {
EXPECT_EQ(nodes[(leaves_sequence[i+1])-1], cst_->suffix_link(nodes[(leaves_sequence[i])-1]));
}


std::vector<int> nodes_sequence = {21, 41, 19, 39, 17, 37, 15, 35, 13, 33, 11, 31, 9, 29, 7, 27, 5, 25, 3, 1, 1};
for (int i = 0; i < nodes_sequence.size()-1; ++i) {
EXPECT_EQ(nodes[(nodes_sequence[i+1])-1], cst_->suffix_link(nodes[(nodes_sequence[i])-1]));
}

}

/*
// This test weiner link on every_node for its correspondent suffix link, then a sampling, then nonexistent links
TEST_P(CST_SCT3_RLCSAFixture, weiner_link_check) {
    std::vector<std::pair<int,int>> leaves_sequence = {{'h',24}, {'b', 23}, {'a', 43}, {'n', 22}, {'a', 42}, {'n', 20}, {'a', 40}, {'n', 18}, {'a', 38}, {'n', 16} ,{'a', 36}, {'n', 14}, {'a', 34}, {'n', 12}, {'a', 32}, {'n', 10}, {'a', 30}, {'n', 8}, {'a', 28}, {'n', 6}, {'a', 26}, {'n', 4}, {'a',2}, {'$', 1}};
    for (int i = leaves_sequence.size()-1; i > 0; --i) {
        EXPECT_EQ(cst_->pbtcTree_->node_select(leaves_sequence[i-1].second), cst_->weiner_link(cst_->pbtcTree_->node_select(leaves_sequence[i].second), leaves_sequence[i].first));
    }

    std::vector<std::pair<int,int>> nodes_sequence = {{'n', 21}, {'a', 41}, {'n', 19}, {'a', 39}, {'n', 17}, {'a', 37}, {'n', 15}, {'a', 35}, {'n', 13}, {'a', 33}, {'n', 11}, {'a', 31}, {'n', 9}, {'a', 29}, {'n', 7}, {'a', 27}, {'n', 5}, {'a', 25}, {'n',3}, {'a', 1}};
    for (int i = nodes_sequence.size()-1; i > 0; --i) {
        EXPECT_EQ(cst_->pbtcTree_->node_select(nodes_sequence[i-1].second), cst_->weiner_link(cst_->pbtcTree_->node_select(nodes_sequence[i].second), nodes_sequence[i].first));
    }


    EXPECT_EQ(cst_->pbtcTree_->node_select(24),
              cst_->weiner_link(cst_->pbtcTree_->node_select(23), 'b'));

    EXPECT_EQ(0,
              cst_->weiner_link(cst_->pbtcTree_->node_select(9), 'a'));
}
*/


//This method check the locus method with some samples
TEST_P(CST_SCT3_RLCSAFixture, locus_check) {
EXPECT_EQ(nodes[5-1],
        cst_->locus("ana"));
EXPECT_EQ(nodes[24-1],
        cst_->locus("b"));
EXPECT_EQ(nodes[27-1],
        cst_->locus("nana"));
EXPECT_EQ(sdsl::bp_interval<uint64_t>(-1,-1,0,0),
cst_->locus("bna"));
}


//This method check the string_ancestor method with some samples
TEST_P(CST_SCT3_RLCSAFixture, string_ancestor_check) {
EXPECT_EQ(nodes[5-1],
        cst_->string_ancestor(nodes[13-1], 3));
cst_->string_ancestor(nodes[13-1], 3);

EXPECT_EQ(nodes[27-1],
        cst_->string_ancestor(nodes[32-1], 4));


EXPECT_EQ(nodes[24-1],
        cst_->string_ancestor(nodes[24-1], 4));
}