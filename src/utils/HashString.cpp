//
// Created by sking32 on 3/14/18.
//

#include "blocktree.utils/HashString.h"

#include <iostream>

HashString::HashString(size_t hash, std::string& s, int init, int end) : hash_(hash), s_(s), init_(init), end_(end), ws_(waux){

}

HashString::HashString(size_t hash, std::basic_string<int64_t>& s, int init, int end) : hash_(hash), ws_(s), init_(init), end_(end), s_(aux){

}

HashString::~HashString() {

}

bool HashString::operator==(const HashString &other) const {
    int length = end_-init_+1;
    if (length != other.end_-other.init_+1) return false;
    if (s_.size () > 0) {
        for (int i = 0; i < length; ++i) {
            if (s_[init_ + i] != other.s_[other.init_ + i]) return false;
        }
        return true;
    }

    for (int i = 0; i < length; ++i) {
        if (ws_[init_ + i] != other.ws_[other.init_ + i]) return false;
    }
    return true;

}