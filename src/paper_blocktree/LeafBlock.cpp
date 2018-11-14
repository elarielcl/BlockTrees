//
// Created by sking32 on 3/9/18.
//

#include <iostream>

#include "paper_blocktree/LeafBlock.h"

LeafBlock::LeafBlock(Block* parent, int start_index, int end_index, int r, int max_leaf_length, std::string& source, int child_number):
        Block(parent, start_index, end_index, r, max_leaf_length, source, child_number), data_(source.substr(start_index, end_index-start_index+1)) {
}

LeafBlock::~LeafBlock() {

}

int LeafBlock::add_rank_select_leaf_support() {
    starts_with_end_leaf_ = source_[start_index_] != source_[0] && source_[start_index_-1] == source_[0];
    leaf_rank_ = (starts_with_end_leaf_) ? 1 : 0;
    std::string rep = represented_string();
    bool one_seen = false;

    for (char c: rep) {
        if (c == source_[0]) {
            one_seen = true;
        }
        else  {
            if (one_seen) {
                ++leaf_rank_;
            }
            one_seen = false;
        }
    }

    return leaf_rank_;
}

int LeafBlock::add_rank_select_support(int c) {
    ranks_[c] = rank(c, length()-1);
    return ranks_[c];
}


int LeafBlock::leaf_rank(int i) {
    if (i == -1) return 0;
    int r = 0;
    bool one_seen = starts_with_end_leaf_;
    for (int j = 0; j<=i; ++j) {
        if (data_[j] == source_[0]) {
            one_seen = true;
        } else {
            if (one_seen) {
                ++r;
            }
            one_seen = false;
        }
    }
    return r;
}

int LeafBlock::leaf_rank_alternative(int i) {
    if (i == -1) return 0;
    int r = 0;
    bool one_seen = starts_with_end_leaf_;
    for (int j = 0; j<=i; ++j) {
        if (data_[j] == source_[0]) {
            one_seen = true;
        } else {
            if (one_seen) {
                ++r;
            }
            one_seen = false;
        }
    }
    return r;
}

int LeafBlock::better_leaf_rank(int i) {
    if (i == -1) return 0;
    int r = 0;
    bool one_seen = starts_with_end_leaf_;
    for (int j = 0; j<=i; ++j) {
        if (data_[j] == source_[0]) {
            one_seen = true;
        } else {
            if (one_seen) {
                ++r;
            }
            one_seen = false;
        }
    }
    return r;
}



int LeafBlock::rank(int c, int i) {
    int r = 0;
    for (int j = 0; j<=i; ++j) {
        if (data_[j] == c) ++r;
    }
    return r;
}

int LeafBlock::rank_alternative(int c, int i) {
    int r = 0;
    for (int j = 0; j<=i; ++j) {
        if (data_[j] == c) ++r;
    }
    return r;
}

int LeafBlock::better_rank(int c, int i) {
    int r = 0;
    for (int j = 0; j<=i; ++j) {
        if (data_[j] == c) ++r;
    }
    return r;
}

int LeafBlock::leaf_select(int j) {
    if (starts_with_end_leaf_ && j == 1) return -1;
    bool one_seen = starts_with_end_leaf_;
    for (int i = 0; i < data_.size(); ++i) {
        if (data_[i] == source_[0]) {
            one_seen = true;
        } else {
            if (one_seen) {
                --j;
            }
            one_seen = false;
        }
        if (!j) return i-1;
    }
    return -1;
}


int LeafBlock::leaf_select_alternative(int j) {
    if (starts_with_end_leaf_ && j == 1) return -1;
    bool one_seen = starts_with_end_leaf_;
    for (int i = 0; i < data_.size(); ++i) {
        if (data_[i] == source_[0]) {
            one_seen = true;
        } else {
            if (one_seen) {
                --j;
            }
            one_seen = false;
        }
        if (!j) return i-1;
    }
    return -1;
}

int LeafBlock::better_leaf_select(int j) {
    if (starts_with_end_leaf_ && j == 1) return -1;
    bool one_seen = starts_with_end_leaf_;
    for (int i = 0; i < data_.size(); ++i) {
        if (data_[i] == source_[0]) {
            one_seen = true;
        } else {
            if (one_seen) {
                --j;
            }
            one_seen = false;
        }
        if (!j) return i-1;
    }
    return -1;
}

int LeafBlock::select(int c, int j) {
    for (int i = 0; i < data_.size(); ++i) {
        if (((int)(data_[i])) == c) --j;
        if (!j) return i;
    }
    return -1;
}


int LeafBlock::select_alternative(int c, int j) {
    for (int i = 0; i < data_.size(); ++i) {
        if (((int)(data_[i])) == c) --j;
        if (!j) return i;
    }
    return -1;
}

int LeafBlock::better_select(int c, int j) {
    for (int i = 0; i < data_.size(); ++i) {
        if (((int)(data_[i])) == c) --j;
        if (!j) return i;
    }
    return -1;
}

void LeafBlock::print() {
    std::cout << "\"" << data_ << "\" ";
}

int LeafBlock::access(int i) {
    return data_[i];
}

int LeafBlock::access_2(int i, int& a) {
    return data_[i];
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