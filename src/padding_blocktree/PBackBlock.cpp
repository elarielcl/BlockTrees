//
// Created by sking32 on 5/18/18.
//

#include <iostream>
#include "padding_blocktree/PBackBlock.h"


PBackBlock::PBackBlock(PBlock* parent, int64_t start_index, int64_t end_index, int r, int leaf_length, std::string& source, int child_number, PBlock* first_block,
                       PBlock* second_block, int offset) :
        PBlock(parent, start_index, end_index, r, leaf_length, source, child_number) {
    first_block_ = first_block;
    if (second_block != nullptr)
        if (second_block->start_index_ == start_index && second_block->end_index_ == end_index) second_block_ = this;
        else second_block_ = second_block;
    offset_ = offset;
    if (first_block_ != nullptr) first_block_->pointing_to_me_ = first_block_->pointing_to_me_ + 1;
    if (second_block_ != nullptr) second_block_->pointing_to_me_ = second_block_->pointing_to_me_ + 1;
}

PBackBlock::~PBackBlock() {
    if (first_block_ != nullptr) {
        first_block_->pointing_to_me_ = first_block_->pointing_to_me_ - 1;
        if (first_block_->pointing_to_me_ == 0)
            first_block_->color_ = 0;
    }
    if (second_block_ != nullptr) {
        second_block_->pointing_to_me_ = second_block_->pointing_to_me_ - 1;
        if (second_block_->pointing_to_me_ == 0)
            second_block_->color_ = 0;
    }
}

int PBackBlock::add_rank_select_support(int c) {
    int first_rank = first_block_->rank(c, offset_-1);
    int second_rank = (second_block_ == nullptr) ? first_block_->rank(c, offset_ + length() - 1) - first_rank : first_block_->rank(c, first_block_->length() - 1) - first_rank;
    first_ranks_[c] = first_rank;
    second_ranks_[c] = second_rank;
    ranks_[c] = (second_block_ == nullptr) ? second_rank : second_rank + second_block_->rank(c, offset_+length()-1-first_block_->length());
    return ranks_[c];
}

int PBackBlock::test_rank(int c, int i, int& counter) {
    ++counter;
    if (i + offset_ >= first_block_->length()) return second_ranks_[c] + second_block_->test_rank(c, offset_+i-first_block_->length(),counter); //Loop if it's itself
    return first_block_->test_rank(c, i+offset_,counter) - first_ranks_[c];
}

int PBackBlock::rank(int c, int i) {
    if (i + offset_ >= first_block_->length()) return second_ranks_[c] + second_block_->rank(c, offset_+i-first_block_->length()); //Loop if it's itself
    return first_block_->rank(c, i+offset_) - first_ranks_[c];
}

int PBackBlock::rank_alternative(int c, int i) {
    if (i + offset_ >= first_block_->length()) return second_ranks_[c] + second_block_->rank_alternative(c, offset_+i-first_block_->length()); //Loop if it's itself
    return first_block_->rank_alternative(c, i+offset_) - (first_block_->ranks_[c] - second_ranks_[c]);
}

int PBackBlock::better_rank(int c, int i) {
    if (i + offset_ >= first_block_->length()) return second_ranks_[c] + second_block_->better_rank(c, offset_+i-first_block_->length()); //Loop if it's itself
    return first_block_->better_rank(c, i+offset_) - (((first_block_->child_number_ == 0) ? first_block_->cumulated_ranks_[c] :
                                                            (first_block_->cumulated_ranks_[c] - first_block_->parent_->children_[first_block_->child_number_-1]->cumulated_ranks_[c])) - second_ranks_[c]);
}

int PBackBlock::select(int c, int j) {
    if (j > second_ranks_[c]) return second_block_->select(c, j-second_ranks_[c]) + first_block_->length() - offset_;
    return first_block_->select(c, j+first_ranks_[c]) - offset_;
}

int PBackBlock::select_alternative(int c, int j) {
    if (j > second_ranks_[c]) return second_block_->select_alternative(c, j-second_ranks_[c]) + first_block_->length() - offset_;
    return first_block_->select_alternative(c, j+first_block_->ranks_[c] - second_ranks_[c]) - offset_;
}

int PBackBlock::better_select(int c, int j) {
    if (j > second_ranks_[c]) return second_block_->better_select(c, j-second_ranks_[c]) + first_block_->length() - offset_;
    return first_block_->better_select(c, j+((first_block_->child_number_ == 0) ? first_block_->cumulated_ranks_[c] :
                                                  (first_block_->cumulated_ranks_[c] - first_block_->parent_->children_[first_block_->child_number_-1]->cumulated_ranks_[c])) - second_ranks_[c]) - offset_;
}

void PBackBlock::print() {
    std::cout << " ( (" << start_index_ << "," << end_index_ << ") ->" ;
    first_block_->print();
    if (second_block_ != nullptr) {
        std::cout << " & ";
        if (second_block_ != this ) second_block_->print();
        else std::cout << " (me) ";
    }
    std::cout << "by " << offset_ << " ) ";
}

int PBackBlock::height() {
    return first_block_->height();
}

int PBackBlock::access(int i) {
    if (i + offset_ >= first_block_->length()) return second_block_->access(offset_+i-first_block_->length()); //Loop if it's itself
    return first_block_->access(i+offset_);
}


int PBackBlock::access_2(int i, int& a) {
    a++;
    if (i + offset_ >= first_block_->length()) return second_block_->access_2(offset_+i-first_block_->length(), a); //Loop if it's itself
    return first_block_->access_2(i+offset_, a);
}

int PBackBlock::number_of_selfreferences() {
    if (this == second_block_) return 1;
    return 0;
}

bool PBackBlock::check_heuristic() {
    if (offset_ >= first_block_->length())
        return false;
    return first_block_->start_index_ < start_index_;
}

bool PBackBlock::check() {
    if (offset_ >= first_block_->length())
        return false;
    if (!(first_block_->start_index_ < start_index_ && second_block_ != this))
        return false;
    return first_block_->start_index_ < start_index_ && second_block_ != this;
}

bool PBackBlock::contains_double_pointer(bool from_back) {
    if (from_back) return true;
    bool first  = first_block_->contains_double_pointer(true);
    if (second_block_ == nullptr) return first;
    return first || second_block_->contains_double_pointer(true);
}

int PBackBlock::number_of_back_blocks() {
    return 1;
}

int PBackBlock::compute_max_hop() {
    int pointed_max_hop = first_block_->max_hop_;
    if (second_block_ != nullptr) pointed_max_hop = (pointed_max_hop > second_block_->max_hop_) ? pointed_max_hop : second_block_->max_hop_;
    max_hop_ = pointed_max_hop + 1;
    return max_hop_;
}

void PBackBlock::put_fix_references() {
    fix_reference_ = first_block_->start_index_;
}