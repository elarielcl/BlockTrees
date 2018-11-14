//
// Created by sking32 on 3/13/18.
//

#include "../../includes/blocktree.utils/BackwardRabinKarp.h"


BackwardRabinKarp::BackwardRabinKarp(std::string& s, int init, int size, int range, int sigma) : sigma_(sigma), size_(size), s_(s), hash_(0), init_(init), rm_(1), kp_(range) {
    for (int i = init_+size_-1; i >= init_; --i) {
        hash_ = (sigma * hash_ + s_[i]) % kp_; // sigma or  little prime
    }

    for (int i = 0; i < size_-1; ++i) {
        rm_ = (rm_ * sigma_) % kp_;
    }
}

int BackwardRabinKarp::hash() {
    return hash_;
}

void BackwardRabinKarp::next() {
    hash_ = (hash_ + kp_ - rm_*s_[init_+size_-1] % kp_) % kp_;
    --init_;
    hash_ = (hash_*sigma_ + s_[init_]) % kp_;
}
