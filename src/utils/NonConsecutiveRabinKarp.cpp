//
// Created by sking32 on 4/4/18.
//

#include "../../includes/blocktree.utils/NonConsecutiveRabinKarp.h"

NonConsecutiveRabinKarp::NonConsecutiveRabinKarp(std::string& s, int init, int size, int range, int sigma) : sigma_(sigma), size_(size),hash_(0), rm_(1), kp_(range) {
    for (int i = init; i < init+size_; ++i) {
        hash_ = (sigma_ * hash_ + s[i]+128) % kp_; // sigma or  little prime
        elements_.push(s[i]);
    }

    for (int i = 0; i < size_-1; ++i) {
        rm_ = (rm_ * sigma_) % kp_;
    }
}

NonConsecutiveRabinKarp::NonConsecutiveRabinKarp(std::basic_string<int64_t>& s, int init, int size, int range, int sigma) : wsigma_(sigma), size_(size),whash_(0), wrm_(1), wkp_(range) {
    for (int i = init; i < init+size_; ++i) {
        uint64_t next = s[i];
        next = next%wkp_;
        whash_ = (wsigma_ * whash_ + next) % wkp_; // sigma or  little prime
        welements_.push(next);
    }

    for (int i = 0; i < size_-1; ++i) {
        wrm_ = (wrm_ * wsigma_) % wkp_;
    }
}

uint64_t NonConsecutiveRabinKarp::hash() {
    if (elements_.size() > 0) return hash_;
    return whash_;
}

void NonConsecutiveRabinKarp::put(int c) {
    int removed = elements_.front()+128;
    elements_.pop();
    elements_.push(c);
    hash_ = (hash_ + kp_ - rm_ * removed % kp_) % kp_;
    hash_ = (hash_ * sigma_ + c+128) % kp_;
}

void NonConsecutiveRabinKarp::wput(uint64_t c) {
    uint64_t removed = welements_.front();
    removed = removed%wkp_;
    welements_.pop();
    c = c%wkp_;
    welements_.push(c);
    whash_ = (whash_ + wkp_ - wrm_ * removed % wkp_) % wkp_;
    whash_ = (whash_ * wsigma_ + c) % wkp_;
}

std::string NonConsecutiveRabinKarp::represented_string() {
    std::string ret = "";
    for (int i = 0; i<size_; ++i) {
        char c = elements_.front();
        elements_.pop();
        elements_.push(c);
        ret += c;
    }
    return ret;

}