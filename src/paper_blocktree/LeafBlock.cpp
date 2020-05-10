//
// Created by sking32 on 3/9/18.
//

#include <iostream>

#include "paper_blocktree/LeafBlock.h"

LeafBlock::LeafBlock(Block* parent, int start_index, int end_index, int r, int max_leaf_length, std::string& source, int child_number):
        Block(parent, start_index, end_index, r, max_leaf_length, source, child_number) {
    size =  source.substr(start_index, end_index-start_index+1).size();
}

LeafBlock::~LeafBlock() {

}

int LeafBlock::add_rank_select_support(int c) {
    ranks_[c] = rank(c, length()-1);
    return ranks_[c];
}

int LeafBlock::rank(int c, int i) {
    int r = 0;
    for (int j = 0; j<=i; ++j) {
        if (source_[start_index_+j] == c) ++r;
    }
    return r;
}

int LeafBlock::rank_alternative(int c, int i) {
    int r = 0;
    for (int j = 0; j<=i; ++j) {
        if (source_[start_index_+j] == c) ++r;
    }
    return r;
}

int LeafBlock::better_rank(int c, int i) {
    int r = 0;
    for (int j = 0; j<=i; ++j) {
        if (source_[start_index_+j] == c) ++r;
    }
    return r;
}

int LeafBlock::select(int c, int j) {
    for (int i = 0; i < size; ++i) {
        if (((int)(source_[start_index_+i])) == c) --j;
        if (!j) return i;
    }
    return -1;
}


int LeafBlock::select_alternative(int c, int j) {
    for (int i = 0; i < size; ++i) {
        if (((int)(source_[start_index_+i])) == c) --j;
        if (!j) return i;
    }
    return -1;
}

int LeafBlock::better_select(int c, int j) {
    for (int i = 0; i < size; ++i) {
        if (((int)(source_[start_index_+i])) == c) --j;
        if (!j) return i;
    }
    return -1;
}

void LeafBlock::print() {
    std::cout << "\"" << source_.substr(start_index_, size) << "\" ";
}

int LeafBlock::access(int i) {
    return source_[start_index_+i];
}

int LeafBlock::access_2(int i, int& a) {
    return source_[start_index_+i];
}

bool LeafBlock::check_heuristic() {
    int l = length();
    return l<r_ || l<=max_leaf_length_;

}

bool LeafBlock::check() {
    int l = length();
    return l<r_ || l<=max_leaf_length_;
}

int LeafBlock::number_of_leaf_blocks() {
    return 1;
}