//
// Created by sking32 on 4/10/18.
//

#include "blocktree.utils/NonConsecutiveHashString.h"

#include <iostream>

NonConsecutiveHashString::NonConsecutiveHashString(size_t hash, int first_block_start_index, int second_block_start_index, int first_block_length, std::string& source, int offset, int size): hash_(hash), first_block_start_index_(first_block_start_index), second_block_start_index_(second_block_start_index), first_block_length_(first_block_length), source_(source), offset_(offset), size_(size),wsource_(waux){

}

NonConsecutiveHashString::NonConsecutiveHashString(size_t hash, int first_block_start_index, int second_block_start_index, int first_block_length, std::basic_string<int64_t>& source, int offset, int size): hash_(hash), first_block_start_index_(first_block_start_index), second_block_start_index_(second_block_start_index), first_block_length_(first_block_length), wsource_(source), offset_(offset), size_(size),source_(aux){

}

NonConsecutiveHashString::~NonConsecutiveHashString() {

}

bool NonConsecutiveHashString::operator==(const NonConsecutiveHashString &other) const {
    if (size_ != other.size_) return false;

    if (source_.size() > 0) {
        std::string &source = source_;
        int i = 0;
        for (int i = 0; i < size_; ++i) {
            int c1 = 0;
            int c2 = 0;
            if (offset_ + i < first_block_length_)
                c1 = source[first_block_start_index_ + offset_ + i];
            else
                c1 = source[second_block_start_index_ + (i - (first_block_length_ - offset_))];

            if (other.offset_ + i < other.first_block_length_)
                c2 = source[other.first_block_start_index_ + other.offset_ + i];
            else
                c2 = source[other.second_block_start_index_ + (i - (other.first_block_length_) - other.offset_)];

            if (c1 != c2) return false;
        }
        return true;
    }

    std::basic_string<int64_t> &source = wsource_;
    int i = 0;
    for (int i = 0; i < size_; ++i) {
        int64_t c1 = 0;
        int64_t c2 = 0;
        if (offset_ + i < first_block_length_)
            c1 = source[first_block_start_index_ + offset_ + i];
        else
            c1 = source[second_block_start_index_ + (i - (first_block_length_ - offset_))];

        if (other.offset_ + i < other.first_block_length_)
            c2 = source[other.first_block_start_index_ + other.offset_ + i];
        else
            c2 = source[other.second_block_start_index_ + (i - (other.first_block_length_) - other.offset_)];

        if (c1 != c2)
            return false;
    }
    return true;
}