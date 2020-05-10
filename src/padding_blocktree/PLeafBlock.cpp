//
// Created by sking32 on 5/18/18.
//

#include <iostream>

#include "padding_blocktree/PLeafBlock.h"

PLeafBlock::PLeafBlock(PBlock* parent, int64_t start_index, int64_t end_index, int r, int leaf_length, std::string& source, int child_number):
        PBlock(parent, start_index, end_index, r, leaf_length, source, child_number) {
}

PLeafBlock::~PLeafBlock() {

}

int64_t PLeafBlock::size() {
    int64_t source_end_index = source_.size() - 1;
    return (end_index_ <= source_end_index ? end_index_ : source_end_index)-start_index_+1;
}

int PLeafBlock::add_rank_select_support(int c) {
    ranks_[c] = rank(c, size()-1);
    return ranks_[c];
}

int PLeafBlock::test_rank(int c, int i, int& counter) {
    ++counter;
    int r = 0;
    for (int j = 0; j<=i; ++j) {
        if (source_[start_index_+j] == c) ++r;
    }
    return r;
}

int PLeafBlock::rank(int c, int i) {
    int r = 0;
    for (int j = 0; j<=i; ++j) {
        if (source_[start_index_+j] == c) ++r;
    }
    return r;
}

int PLeafBlock::rank_alternative(int c, int i) {
    int r = 0;
    for (int j = 0; j<=i; ++j) {
        if (source_[start_index_+j] == c) ++r;
    }
    return r;
}

int PLeafBlock::better_rank(int c, int i) {
    int r = 0;
    for (int j = 0; j<=i; ++j) {
        if (source_[start_index_+j] == c) ++r;
    }
    return r;
}

int PLeafBlock::select(int c, int j) {
    for (int i = 0; i < size(); ++i) {
        if (((int)(source_[start_index_+i])) == c) --j;
        if (!j) return i;
    }
    return -1;
}


int PLeafBlock::select_alternative(int c, int j) {
    for (int i = 0; i < size(); ++i) {
        if (((int)(source_[start_index_+i])) == c) --j;
        if (!j) return i;
    }
    return -1;
}

int PLeafBlock::better_select(int c, int j) {
    for (int i = 0; i < size(); ++i) {
        if (((int)(source_[start_index_+i])) == c) --j;
        if (!j) return i;
    }
    return -1;
}

void PLeafBlock::print() {
    std::cout << "\"" << source_.substr(start_index_, size()) << "\" ";
}

int PLeafBlock::access(int i) {
    return source_[start_index_+i];
}

int PLeafBlock::access_2(int i, int& a) {
    return source_[start_index_+i];
}

bool PLeafBlock::check_heuristic() {
    int l = size();
    return l<r_ || l<=leaf_length_;

}

bool PLeafBlock::check() {
    int l = size();
    return l<r_ || l<=leaf_length_;
}

int PLeafBlock::number_of_leaf_blocks() {
    return 1;
}
