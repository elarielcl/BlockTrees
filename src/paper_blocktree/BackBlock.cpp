//
// Created by sking32 on 3/9/18.
//

#include <iostream>
#include "paper_blocktree/BackBlock.h"


BackBlock::BackBlock(Block* parent, int start_index, int end_index, int r, int max_leaf_length, std::string& source, int child_number, Block* first_block,
                     Block* second_block, int offset) :
        Block(parent, start_index, end_index, r, max_leaf_length, source, child_number), suffix_start_with_end_leaf_(false), prefix_start_with_end_leaf_(false) {
    first_block_ = first_block;
    if (second_block != nullptr)
        if (second_block->start_index_ == start_index && second_block->end_index_ == end_index) second_block_ = this;
        else second_block_ = second_block;
    offset_ = offset;
    if (first_block_ != nullptr) first_block_->pointing_to_me_ = first_block_->pointing_to_me_ + 1;
    if (second_block_ != nullptr) second_block_->pointing_to_me_ = second_block_->pointing_to_me_ + 1;
}

BackBlock::~BackBlock() {
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

int BackBlock::add_rank_select_leaf_support() {
    starts_with_end_leaf_ = source_[start_index_] != source_[0] && source_[start_index_-1] == source_[0];
    suffix_start_with_end_leaf_ = source_[first_block_->start_index_+offset_] != source_[0] && source_[first_block_->start_index_+offset_-1] == source_[0];
    prefix_start_with_end_leaf_ = source_[start_index_+first_block_->length()-offset_] != source_[0] && source_[start_index_+first_block_->length()-offset_-1] == source_[0];
    int first_rank = (offset_ == 0) ? 0 : first_block_->leaf_rank(offset_ - 1);
    int second_rank = (second_block_ == nullptr) ?
                      first_block_->leaf_rank(offset_ + length() - 1) - first_rank :
                      first_block_->leaf_rank(first_block_->length() - 1) - first_rank;
    first_leaf_rank_ = first_rank;
    second_leaf_rank_ = second_rank;
    /*
    if (is_leaf_end(first_block_->start_index_ + offset_) && !is_leaf_end(start_index_)) {
        ++first_leaf_rank_;
        --second_leaf_rank_;
    }
    if (!is_leaf_end(first_block_->start_index_ + offset_) && is_leaf_end(start_index_)) {
        --first_leaf_rank_;
        ++second_leaf_rank_;
    }

    if (second_block_ != nullptr) {
        if (is_leaf_end(second_block_->start_index_) && !is_leaf_end(start_index_+first_block_->length()-offset_)) {
            --second_leaf_rank_;
        }
        if (!is_leaf_end(second_block_->start_index_) && is_leaf_end(start_index_+first_block_->length()-offset_)) {
            ++second_leaf_rank_;
        }
    }
    */
    leaf_rank_ = (second_block_ == nullptr) ? second_leaf_rank_ : second_leaf_rank_ + second_block_->leaf_rank(
            offset_ + length() - 1 - first_block_->length());

    if (starts_with_end_leaf_ && !suffix_start_with_end_leaf_) ++leaf_rank_;
    if (!starts_with_end_leaf_ && suffix_start_with_end_leaf_) --leaf_rank_;
    if (second_block_ != nullptr) {
        if (second_block_->starts_with_end_leaf_ && !prefix_start_with_end_leaf_) --leaf_rank_;
        if (!second_block_->starts_with_end_leaf_ && prefix_start_with_end_leaf_) ++leaf_rank_;
    }
    return leaf_rank_;
}


int BackBlock::add_rank_select_support(int c) {
    int first_rank = first_block_->rank(c, offset_-1);
    int second_rank = (second_block_ == nullptr) ? first_block_->rank(c, offset_ + length() - 1) - first_rank : first_block_->rank(c, first_block_->length() - 1) - first_rank;
    first_ranks_[c] = first_rank;
    second_ranks_[c] = second_rank;
    ranks_[c] = (second_block_ == nullptr) ? second_rank : second_rank + second_block_->rank(c, offset_+length()-1-first_block_->length());
    return ranks_[c];
}

int BackBlock::rank(int c, int i) {
    if (i + offset_ >= first_block_->length()) return second_ranks_[c] + second_block_->rank(c, offset_+i-first_block_->length()); //Loop if it's itself
    return first_block_->rank(c, i+offset_) - first_ranks_[c];
}

int BackBlock::rank_alternative(int c, int i) {
    if (!offset_) {
        if (i >= first_block_->length()) return second_ranks_[c] + second_block_->rank_alternative(c, i-first_block_->length()); //Loop if it's itself
        return first_block_->rank_alternative(c, i);
    }
    if (i + offset_ >= first_block_->length()) return second_ranks_[c] + second_block_->rank_alternative(c, offset_+i-first_block_->length()); //Loop if it's itself
    return first_block_->rank_alternative(c, i+offset_) - (first_block_->ranks_[c] - second_ranks_[c]);
}

int BackBlock::better_rank(int c, int i) {
    if (!offset_) {
        if (i >= first_block_->length()) return second_ranks_[c] + second_block_->better_rank(c, i-first_block_->length()); //Loop if it's itself
        return first_block_->better_rank(c, i);
    }
    if (i + offset_ >= first_block_->length()) return second_ranks_[c] + second_block_->better_rank(c, offset_+i-first_block_->length()); //Loop if it's itself
    return first_block_->better_rank(c, i+offset_) - (((first_block_->child_number_ == 0) ? first_block_->cumulated_ranks_[c] :
                                                       (first_block_->cumulated_ranks_[c] - first_block_->parent_->children_[first_block_->child_number_-1]->cumulated_ranks_[c])) - second_ranks_[c]);
}

int BackBlock::select(int c, int j) {
    if (j > second_ranks_[c]) return second_block_->select(c, j-second_ranks_[c]) + first_block_->length() - offset_;
    return first_block_->select(c, j+first_ranks_[c]) - offset_;
}

int BackBlock::select_alternative(int c, int j) {
    if (offset_ == 0) {
        if (j > second_ranks_[c]) return second_block_->select_alternative(c, j-second_ranks_[c]) + first_block_->length();
        return first_block_->select_alternative(c, j);
    }
    if (j > second_ranks_[c]) return second_block_->select_alternative(c, j-second_ranks_[c]) + first_block_->length() - offset_;
    return first_block_->select_alternative(c, j+first_block_->ranks_[c] - second_ranks_[c]) - offset_;
}

int BackBlock::better_select(int c, int j) {
    if (offset_ == 0) {
        if (j > second_ranks_[c]) return second_block_->better_select(c, j-second_ranks_[c]) + first_block_->length();
        return first_block_->better_select(c, j);
    }
    if (j > second_ranks_[c]) return second_block_->better_select(c, j-second_ranks_[c]) + first_block_->length() - offset_;
    return first_block_->better_select(c, j+((first_block_->child_number_ == 0) ? first_block_->cumulated_ranks_[c] :
                                             (first_block_->cumulated_ranks_[c] - first_block_->parent_->children_[first_block_->child_number_-1]->cumulated_ranks_[c])) - second_ranks_[c]) - offset_;
}


int BackBlock::leaf_rank(int i) {
    int different_start_value = 0;
    if (starts_with_end_leaf_ && !suffix_start_with_end_leaf_) ++different_start_value;
    if (!starts_with_end_leaf_ && suffix_start_with_end_leaf_) --different_start_value;
    int separated_blocks_value = 0;
    if (second_block_ != nullptr) {
        if (second_block_->starts_with_end_leaf_ && !prefix_start_with_end_leaf_) --separated_blocks_value;
        if (!second_block_->starts_with_end_leaf_ && prefix_start_with_end_leaf_) ++separated_blocks_value;
    }

    if (i + offset_ >= first_block_->length()) return separated_blocks_value + different_start_value + second_leaf_rank_ + second_block_->leaf_rank(offset_+i-first_block_->length()); //Loop if it's itself
    return different_start_value + first_block_->leaf_rank(i+offset_) - first_leaf_rank_;
}

int BackBlock::leaf_rank_alternative(int i) {
    int different_start_value = 0;
    if (starts_with_end_leaf_ && !suffix_start_with_end_leaf_) ++different_start_value;
    if (!starts_with_end_leaf_ && suffix_start_with_end_leaf_) --different_start_value;
    int separated_blocks_value = 0;
    if (second_block_ != nullptr) {
        if (second_block_->starts_with_end_leaf_ && !prefix_start_with_end_leaf_) --separated_blocks_value;
        if (!second_block_->starts_with_end_leaf_ && prefix_start_with_end_leaf_) ++separated_blocks_value;
    }

    if (!offset_) {
        if (i >= first_block_->length()) {
            return different_start_value + separated_blocks_value + second_leaf_rank_ + second_block_->leaf_rank_alternative(i-first_block_->length()); //Loop if it's itself
        }

        return different_start_value + first_block_->leaf_rank_alternative(i);
    }
    if (i + offset_ >= first_block_->length()) return separated_blocks_value+different_start_value+ second_leaf_rank_ + second_block_->leaf_rank_alternative(offset_+i-first_block_->length()); //Loop if it's itself
    return different_start_value + first_block_->leaf_rank_alternative(i+offset_) - (first_block_->leaf_rank_ - second_leaf_rank_);
}

int BackBlock::better_leaf_rank(int i) {
    int different_start_value = 0;
    if (starts_with_end_leaf_ && !suffix_start_with_end_leaf_) ++different_start_value;
    if (!starts_with_end_leaf_ && suffix_start_with_end_leaf_) --different_start_value;
    int separated_blocks_value = 0;
    if (second_block_ != nullptr) {
        if (second_block_->starts_with_end_leaf_ && !prefix_start_with_end_leaf_) --separated_blocks_value;
        if (!second_block_->starts_with_end_leaf_ && prefix_start_with_end_leaf_) ++separated_blocks_value;
    }
    if (!offset_) {
        if (i >= first_block_->length()) return different_start_value + separated_blocks_value + second_leaf_rank_ + second_block_->better_leaf_rank(i-first_block_->length()); //Loop if it's itself
        return different_start_value + first_block_->better_leaf_rank(i);
    }
    if (i + offset_ >= first_block_->length()) return different_start_value + separated_blocks_value + second_leaf_rank_ + second_block_->better_leaf_rank(offset_+i-first_block_->length()); //Loop if it's itself
    return different_start_value + first_block_->better_leaf_rank(i+offset_) - (((first_block_->child_number_ == 0) ? first_block_->cumulated_leaf_rank_ :
                                                       (first_block_->cumulated_leaf_rank_ - first_block_->parent_->children_[first_block_->child_number_-1]->cumulated_leaf_rank_)) - second_leaf_rank_);
}

int BackBlock::leaf_select(int j) {
    int different_start_value = 0;
    if (starts_with_end_leaf_ && !suffix_start_with_end_leaf_) ++different_start_value;
    if (!starts_with_end_leaf_ && suffix_start_with_end_leaf_) --different_start_value;
    int separated_blocks_value = 0;
    if (second_block_ != nullptr) {
        if (second_block_->starts_with_end_leaf_ && !prefix_start_with_end_leaf_) --separated_blocks_value;
        if (!second_block_->starts_with_end_leaf_ && prefix_start_with_end_leaf_) ++separated_blocks_value;
    }

    if (j > different_start_value + second_leaf_rank_) {
        if (j - different_start_value - second_leaf_rank_ == 1 && separated_blocks_value == 1) return first_block_->length()-offset_-1;
        return second_block_->leaf_select(j-second_leaf_rank_-different_start_value-separated_blocks_value) + first_block_->length() - offset_;
    }

    if (j == different_start_value) return -1;
    return first_block_->leaf_select(j+first_leaf_rank_-different_start_value) - offset_;
}

int BackBlock::leaf_select_alternative(int j) {
    int different_start_value = 0;
    if (starts_with_end_leaf_ && !suffix_start_with_end_leaf_) ++different_start_value;
    if (!starts_with_end_leaf_ && suffix_start_with_end_leaf_) --different_start_value;
    int separated_blocks_value = 0;
    if (second_block_ != nullptr) {
        if (second_block_->starts_with_end_leaf_ && !prefix_start_with_end_leaf_) --separated_blocks_value;
        if (!second_block_->starts_with_end_leaf_ && prefix_start_with_end_leaf_) ++separated_blocks_value;
    }

    if (offset_ == 0) {
        if (j > different_start_value + second_leaf_rank_){
            if (j - different_start_value - second_leaf_rank_ == 1 && separated_blocks_value == 1) return first_block_->length()-offset_-1;
            return second_block_->leaf_select_alternative(j-second_leaf_rank_-different_start_value-separated_blocks_value) + first_block_->length();
        }
        if (j == different_start_value) return -1;
        return first_block_->leaf_select_alternative(j+first_leaf_rank_-different_start_value);
    }


    if (j > different_start_value + second_leaf_rank_) {
        if (j - different_start_value - second_leaf_rank_ == 1 && separated_blocks_value == 1) return first_block_->length()-offset_-1;
        return second_block_->leaf_select_alternative(j-second_leaf_rank_-different_start_value-separated_blocks_value) + first_block_->length() - offset_;
    }

    if (j == different_start_value) return -1;
    return first_block_->leaf_select_alternative(j+(first_block_->leaf_rank_ - second_leaf_rank_)-different_start_value) - offset_;
}

int BackBlock::better_leaf_select(int j) {
    int different_start_value = 0;
    if (starts_with_end_leaf_ && !suffix_start_with_end_leaf_) ++different_start_value;
    if (!starts_with_end_leaf_ && suffix_start_with_end_leaf_) --different_start_value;
    int separated_blocks_value = 0;
    if (second_block_ != nullptr) {
        if (second_block_->starts_with_end_leaf_ && !prefix_start_with_end_leaf_) --separated_blocks_value;
        if (!second_block_->starts_with_end_leaf_ && prefix_start_with_end_leaf_) ++separated_blocks_value;
    }

    if (offset_ == 0) {
        if (j > different_start_value + second_leaf_rank_){
            if (j - different_start_value - second_leaf_rank_ == 1 && separated_blocks_value == 1) return first_block_->length()-offset_-1;
            return second_block_->better_leaf_select(j-second_leaf_rank_-different_start_value-separated_blocks_value) + first_block_->length();
        }
        if (j == different_start_value) return -1;
        return first_block_->better_leaf_select(j+first_leaf_rank_-different_start_value);
    }


    if (j > different_start_value + second_leaf_rank_) {
        if (j - different_start_value - second_leaf_rank_ == 1 && separated_blocks_value == 1) return first_block_->length()-offset_-1;
        return second_block_->better_leaf_select(j-second_leaf_rank_-different_start_value-separated_blocks_value) + first_block_->length() - offset_;
    }

    if (j == different_start_value) return -1;
    return first_block_->better_leaf_select(j+(((first_block_->child_number_ == 0) ? first_block_->cumulated_leaf_rank_ :
                                                (first_block_->cumulated_leaf_rank_ - first_block_->parent_->children_[first_block_->child_number_-1]->cumulated_leaf_rank_)) - second_leaf_rank_)-different_start_value) - offset_;
}

void BackBlock::print() {
    std::cout << " ( (" << start_index_ << "," << end_index_ << ") ->" ;
    first_block_->print();
    if (second_block_ != nullptr) {
        std::cout << " & ";
        if (second_block_ != this ) second_block_->print();
        else std::cout << " (me) ";
    }
    std::cout << "by " << offset_ << " ) ";
}

int BackBlock::height() {
    return first_block_->height();
}

int BackBlock::access(int i) {
    if (i + offset_ >= first_block_->length()) return second_block_->access(offset_+i-first_block_->length()); //Loop if it's itself
    return first_block_->access(i+offset_);
}

int BackBlock::access_2(int i, int& a) {
    a++;
    if (i + offset_ >= first_block_->length()) return second_block_->access_2(offset_+i-first_block_->length(), a); //Loop if it's itself
    return first_block_->access_2(i+offset_, a);
}

int BackBlock::number_of_selfreferences() {
    if (this == second_block_) return 1;
    return 0;
}

bool BackBlock::check_heuristic() {
    if (offset_ >= first_block_->length())
        return false;
    return first_block_->start_index_ < start_index_;
}

bool BackBlock::check() {
    if (offset_ >= first_block_->length())
        return false;
    if (!(first_block_->start_index_ < start_index_ && second_block_ != this))
        return false;
    return first_block_->start_index_ < start_index_ && second_block_ != this;
}

bool BackBlock::contains_double_pointer(bool from_back) {
    if (from_back) return true;
    bool first  = first_block_->contains_double_pointer(true);
    if (second_block_ == nullptr) return first;
    return first || second_block_->contains_double_pointer(true);
}

int BackBlock::number_of_back_blocks() {
    return 1;
}

int BackBlock::compute_max_hop() {
    int pointed_max_hop = first_block_->max_hop_;
    if (second_block_ != nullptr) pointed_max_hop = (pointed_max_hop > second_block_->max_hop_) ? pointed_max_hop : second_block_->max_hop_;
    max_hop_ = pointed_max_hop + 1;
    return max_hop_;
}

void BackBlock::put_fix_references() {
    fix_reference_ = first_block_->start_index_;
}