//
// Created by sking32 on 3/12/18.
//

#include <iostream>
#include "../../includes/blocktree.utils/RabinKarp.h"


RabinKarp::RabinKarp(std::string& s, int init, int size, int range, int sigma) : sigma_(sigma), size_(size), s_(s), hash_(0), init_(init), rm_(1), kp_(range) ,ws_(waux){
    for (int i = init; i < init+size_; ++i) {
        hash_ = (sigma_ * hash_ + s_[i]+128) % kp_; // sigma or  little prime
    }

    for (int i = 0; i < size_-1; ++i) {
        rm_ = (rm_ * sigma_) % kp_;
    }
}

RabinKarp::RabinKarp(std::basic_string<int64_t>& s, int init, int size, int range, int sigma) : sigma_(sigma), size_(size), ws_(s), hash_(0), init_(init), rm_(1), kp_(range), s_(aux) {
    for (int i = init; i < init+size_; ++i) {
        uint64_t next = ws_[i];
        next = next%kp_;
        hash_ = (sigma_ * hash_ + next) % kp_; // sigma or  little prime
        //if (hash_ < 0) hash_ += kp_;
        //if (hash_ < 0) std::cout << "DFGD"<<std::endl;
    }

    for (int i = 0; i < size_-1; ++i) {
        rm_ = (rm_ * sigma_) % kp_;
    }
}

uint64_t RabinKarp::hash() {
    return hash_;
}

void RabinKarp::next() {
    hash_ = (hash_ + kp_ - rm_*(s_[init_]+128) % kp_) % kp_;
    init_++;
    hash_ = (hash_*sigma_ + s_[init_+size_-1]+128) % kp_;
}


void RabinKarp::wnext() {
    uint64_t next = ws_[init_];
    next = next%kp_;
    hash_ = (hash_ + kp_ - rm_*next % kp_) % kp_;
    init_++;
    next = ws_[init_+size_-1];
    next = next%kp_;
    hash_ = (hash_*sigma_ + next) % kp_;
    //if (hash_ < 0) hash_ += kp_;
}
