//
// Created by sking32 on 5/18/18.
//

#include <iostream>
#include "padding_blocktree/PBackBlock.h"


PBackBlock::PBackBlock(PBlock* parent, int64_t start_index, int64_t end_index, int r, int leaf_length, std::string& source, int child_number, PBlock* first_block,
                       PBlock* second_block, int offset) :
        PBlock(parent, start_index, end_index, r, leaf_length, source, child_number), suffix_start_with_end_leaf_(false), prefix_start_with_end_leaf_(false),
        min_in_both_blocks_(true), min_in_first_block_(true), max_in_first_block_(true), max_in_both_blocks_(true) {
    first_block_ = first_block;
    if (second_block != nullptr)
        if (second_block->start_index_ == start_index && second_block->end_index_ == end_index) second_block_ = this;
        else second_block_ = second_block;
    offset_ = offset;
    if (first_block_ != nullptr) first_block_->pointing_to_me_ = first_block_->pointing_to_me_ + 1;
    if (second_block_ != nullptr) second_block_->pointing_to_me_ = second_block_->pointing_to_me_ + 1;
}

PBackBlock::PBackBlock(PBlock* parent, int64_t start_index, int64_t end_index, int r, int leaf_length, std::basic_string<int64_t>& source, int child_number, PBlock* first_block,
                       PBlock* second_block, int offset) :
        PBlock(parent, start_index, end_index, r, leaf_length, source, child_number), suffix_start_with_end_leaf_(false), prefix_start_with_end_leaf_(false),
        min_in_both_blocks_(true), min_in_first_block_(true), max_in_first_block_(true), max_in_both_blocks_(true) {
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

int64_t PBackBlock::add_differential_access_support() {
    first_sum_ = first_block_->differential_access(offset_-1);
    second_sum_ = (second_block_ == nullptr) ? first_block_->differential_access(offset_+length()-1) - first_sum_ : first_block_->differential_access(first_block_->length()-1) - first_sum_;
    sum_ = (second_block_ == nullptr) ? second_sum_ : second_sum_ + second_block_->differential_access(offset_+length()-1-first_block_->length());
    return sum_;
}


int PBackBlock::test_rank(int c, int i, int& counter) {
    ++counter;
    if (i + offset_ >= first_block_->length()) return second_ranks_[c] + second_block_->test_rank(c, offset_+i-first_block_->length(),counter); //Loop if it's itself
    return first_block_->test_rank(c, i+offset_,counter) - first_ranks_[c];
}

int64_t PBackBlock::differential_access(int i) {
    if (i + offset_ >= first_block_->length()) return second_sum_ + second_block_->differential_access(offset_+i-first_block_->length()); //Loop if it's itself
    return first_block_->differential_access(i+offset_) - first_sum_;
}

int PBackBlock::rank(int c, int i) {
    if (i + offset_ >= first_block_->length()) return second_ranks_[c] + second_block_->rank(c, offset_+i-first_block_->length()); //Loop if it's itself
    return first_block_->rank(c, i+offset_) - first_ranks_[c];
}

int PBackBlock::rank_alternative(int c, int i) {
    if (i + offset_ >= first_block_->length()) return second_ranks_[c] + second_block_->rank_alternative(c, offset_+i-first_block_->length()); //Loop if it's itself
    return first_block_->rank_alternative(c, i+offset_) - (first_block_->ranks_[c] - second_ranks_[c]);
}

int64_t PBackBlock::differential_access_alternative(int i) {
    if (i + offset_ >= first_block_->length()) return second_sum_ + second_block_->differential_access_alternative(offset_+i-first_block_->length()); //Loop if it's itself
    return first_block_->differential_access_alternative(i+offset_) - (first_block_->sum_ - second_sum_);
}

int64_t PBackBlock::better_differential_access(int i) {
    if (i + offset_ >= first_block_->length()) return second_sum_ + second_block_->better_differential_access(offset_+i-first_block_->length()); //Loop if it's itself
    return first_block_->better_differential_access(i+offset_) - (((first_block_->child_number_ == 0) ? first_block_->cumulated_sum_ :
                                                       (first_block_->cumulated_sum_ - first_block_->parent_->children_[first_block_->child_number_-1]->cumulated_sum_)) - second_sum_);
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

int PBackBlock::waccess(int i) {
    if (i + offset_ >= first_block_->length()) return second_block_->waccess(offset_+i-first_block_->length()); //Loop if it's itself
    return first_block_->waccess(i+offset_);
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

int PBackBlock::add_rank_select_leaf_support() {
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
int PBackBlock::leaf_rank(int i) {
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

int PBackBlock::leaf_rank_alternative(int i) {
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

int PBackBlock::better_leaf_rank(int i) {
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

int PBackBlock::leaf_select(int j) {
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

int PBackBlock::leaf_select_alternative(int j) {
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

int PBackBlock::better_leaf_select(int j) {
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

// Improve this
int PBackBlock::add_fwdsearch_support() {
    int i = 0;
    int min_excess = 1;
    int excess = 0;
    for (; i < length()-offset_; ++i) {
        excess += (source_[i+start_index_] == source_[0]) ? 1 : -1;
        if (excess < min_excess) min_excess = excess;
    }

    int first_min_excess = min_excess;
    int first_excess = excess;
    min_excess = 1;
    excess = 0;
    for (; i < length(); ++i) {
        excess += (source_[i+start_index_] == source_[0]) ? 1 : -1;
        if (excess < min_excess) min_excess = excess;
    }

    int second_excess = excess;
    int second_min_excess = min_excess;

    prefix_first_block_min_excess_ = first_min_excess;
    prefix_second_block_min_excess_ = second_min_excess;

    min_prefix_excess_ = (first_min_excess < first_excess + second_min_excess) ? first_min_excess : (second_min_excess+first_excess);

    if (min_prefix_excess_ < prefix_first_block_min_excess_) {
        min_in_first_block_ = false;
        min_in_both_blocks_ = false;
    } else if (min_prefix_excess_ < first_excess + prefix_second_block_min_excess_) {
        min_in_first_block_ = true;
        min_in_both_blocks_ = false;
    } else {
        min_in_first_block_ = true;
        min_in_both_blocks_ = true;
    }


    return min_prefix_excess_;
}

int PBackBlock::add_preffix_min_count_fields() {
    int i = 0;
    int min_excess = 1;
    int excess = 0;
    int count = 0;
    for (; i < length()-offset_; ++i) {
        excess += (source_[i+start_index_] == source_[0]) ? 1 : -1;
        if (excess == min_excess) ++count;
        if (excess < min_excess) {
            min_excess = excess;
            count = 1;
        }
    }

    int first_min_excess = min_excess;
    int first_excess = excess;
    int first_count = count;

    min_excess = 1;
    excess = 0;
    count = 0;
    for (; i < length(); ++i) {
        excess += (source_[i+start_index_] == source_[0]) ? 1 : -1;
        if (excess == min_excess) ++count;
        if (excess < min_excess) {
            min_excess = excess;
            count = 1;
        }
    }

    int second_excess = excess;
    int second_min_excess = min_excess;
    int second_count = count;

    prefix_first_block_min_excess_ = first_min_excess;
    prefix_second_block_min_excess_ = second_min_excess;
    prefix_min_first_block_count_ = first_count;
    prefix_min_second_block_count_ = second_count;

    min_prefix_excess_ = (first_min_excess < first_excess + second_min_excess) ? first_min_excess : (second_min_excess+first_excess);
    prefix_min_count_ = 0;
    if (first_min_excess == min_prefix_excess_) {
        prefix_min_count_ += prefix_min_first_block_count_;
    }
    if (first_excess + second_min_excess == min_prefix_excess_) {
        prefix_min_count_ += prefix_min_second_block_count_;
    }
    return min_prefix_excess_;
}

int PBackBlock::add_max_fields() {
    int i = 0;
    int max_excess = -1;
    int excess = 0;
    for (; i < length()-offset_; ++i) {
        excess += (source_[i+start_index_] == source_[0]) ? 1 : -1;
        if (excess > max_excess) max_excess = excess;
    }

    int first_max_excess = max_excess;
    int first_excess = excess;
    max_excess = -1;
    excess = 0;
    for (; i < length(); ++i) {
        excess += (source_[i+start_index_] == source_[0]) ? 1 : -1;
        if (excess > max_excess) max_excess = excess;
    }

    int second_excess = excess;
    int second_max_excess = max_excess;

    prefix_first_block_max_excess_ = first_max_excess;
    prefix_second_block_max_excess_ = second_max_excess;

    max_prefix_excess_ = (first_max_excess > first_excess + second_max_excess) ? first_max_excess : (second_max_excess+first_excess);

    if (max_prefix_excess_ > prefix_first_block_max_excess_) {
        max_in_first_block_ = false;
        max_in_both_blocks_ = false;
    } else if (max_prefix_excess_ > first_excess + prefix_second_block_max_excess_) {
        max_in_first_block_ = true;
        max_in_both_blocks_ = false;
    } else {
        max_in_first_block_ = true;
        max_in_both_blocks_ = true;
    }

    return max_prefix_excess_;
}


int PBackBlock::fwdsearch(int i, int d, int& e) {
    int a = -1;
    if (i+1 + offset_ < first_block_->length()) {
        a = first_block_->fwdsearch(i+offset_, d , e);
        if (a != -1) {
            a -= offset_;
            for (auto pair : ranks_) { //Sum Excess
                if (pair.first == source_[0]) e += pair.second;
                else e -= pair.second;
            }
            for (auto pair : second_ranks_) { //Subtract Excess
                if (pair.first == source_[0]) e -= pair.second;
                else e += pair.second;
            }
        } else {
            if (second_block_ != nullptr){ // Be careful on no padding version
                if (e + prefix_second_block_min_excess_ <= d) {
                    a = second_block_->fwdsearch(-1, d, e);
                    a += first_block_->length() - offset_;
                    for (auto pair : ranks_) { //Sum Excess
                        if (pair.first == source_[0]) e += pair.second;
                        else e -= pair.second;
                    }
                    for (auto pair : second_ranks_) { //Subtract Excess
                        if (pair.first == source_[0]) e -= pair.second;
                        else e += pair.second;
                    }

                    for (auto pair : second_block_->ranks_) { //Subtract Excess
                        if (pair.first == source_[0]) e -= pair.second;
                        else e += pair.second;
                    }

                } else {
                    for (auto pair : ranks_) { //Sum Excess
                        if (pair.first == source_[0]) e += pair.second;
                        else e -= pair.second;
                    }
                    for (auto pair : second_ranks_) { //Subtract Excess
                        if (pair.first == source_[0]) e -= pair.second;
                        else e += pair.second;
                    }
                }
            }
        }
    }else {
        a = second_block_->fwdsearch(offset_+i-first_block_->length(), d, e);
        if (a != -1) {
            a += first_block_->length() - offset_;
            if (a >= length()) a = -1; // False positive
            for (auto pair : ranks_) { //Sum Excess
                if (pair.first == source_[0]) e += pair.second;
                else e -= pair.second;
            }
            for (auto pair : second_ranks_) { //Subtract Excess
                if (pair.first == source_[0]) e -= pair.second;
                else e += pair.second;
            }

            for (auto pair : second_block_->ranks_) { //Subtract Excess
                if (pair.first == source_[0]) e -= pair.second;
                else e += pair.second;
            }
        } else {
            for (auto pair : ranks_) { //Sum Excess
                if (pair.first == source_[0]) e += pair.second;
                else e -= pair.second;
            }
            for (auto pair : second_ranks_) { //Subtract Excess
                if (pair.first == source_[0]) e -= pair.second;
                else e += pair.second;
            }

            for (auto pair : second_block_->ranks_) { //Subtract Excess
                if (pair.first == source_[0]) e -= pair.second;
                else e += pair.second;
            }
        }
    }

    return a;
}




int PBackBlock::bwdsearch(int i, int d, int& e) {
    int a = source_.length();
    if (i + offset_ >= first_block_->length()) {
        a = second_block_->bwdsearch(offset_ + i - first_block_->length(), d, e);
        if (a != source_.length()) {
            a += first_block_->length() - offset_;
            for (auto pair : second_ranks_) { //Sum Excess
                if (pair.first == source_[0]) e += pair.second;
                else e -= pair.second;
            }
        } else {
            int excess = 0;
            for (auto pair : second_ranks_) { //Sum Excess
                if (pair.first == source_[0]) excess += pair.second;
                else excess -= pair.second;
            }
            int reached = ((excess - prefix_first_block_min_excess_) > excess) ? (excess - prefix_first_block_min_excess_) : excess;
            if (e + reached >= -d) {
                a = first_block_->bwdsearch(first_block_->length() - 1, d, e); //Secure search
                a -= offset_;
                e += excess;
                for (auto pair : first_block_->ranks_) { //Subtract Excess
                    if (pair.first == source_[0]) e -= pair.second;
                    else e += pair.second;
                }
            } else {
                e += excess;
            }
        }
    } else {
        a = first_block_->bwdsearch(i + offset_, d, e);
        if (a != source_.length()) {
            a -= offset_;
            if (a < -1) {
                a = source_.length(); // False Positive
            }
            for (auto pair : first_block_->ranks_) { //Subtract Excess
                if (pair.first == source_[0]) e -= pair.second;
                else e += pair.second;
            }
            for (auto pair : second_ranks_) { //Sum Excess
                if (pair.first == source_[0]) e += pair.second;
                else e -= pair.second;
            }

        } else {
            for (auto pair : first_block_->ranks_) { //Subtract Excess
                if (pair.first == source_[0]) e -= pair.second;
                else e += pair.second;
            }
            for (auto pair : second_ranks_) { //Sum Excess
                if (pair.first == source_[0]) e += pair.second;
                else e -= pair.second;
            }
        }
    }


    return a;
}


int PBackBlock::positive_bwdsearch(int i, int d, int& e) { //HERE!
    int a = source_.length();
    if (i + offset_ >= first_block_->length()) {
        a = second_block_->positive_bwdsearch(offset_ + i - first_block_->length(), d, e);
        if (a != source_.length()) {
            a += first_block_->length() - offset_;
            for (auto pair : second_ranks_) { //Sum Excess
                if (pair.first == source_[0]) e += pair.second;
                else e -= pair.second;
            }
        } else {
            int excess = 0;
            for (auto pair : second_ranks_) { //Sum Excess
                if (pair.first == source_[0]) excess += pair.second;
                else excess -= pair.second;
            }
            int reached = ((excess - prefix_first_block_max_excess_) < excess) ? (excess - prefix_first_block_max_excess_) : excess;
            if (e + reached <= -d) {
                a = first_block_->positive_bwdsearch(first_block_->length() - 1, d, e); //Secure search
                a -= offset_;
                e += excess;
                for (auto pair : first_block_->ranks_) { //Subtract Excess
                    if (pair.first == source_[0]) e -= pair.second;
                    else e += pair.second;
                }
            } else {
                e += excess;
            }
        }
    } else {
        a = first_block_->positive_bwdsearch(i + offset_, d, e);
        if (a != source_.length()) {
            a -= offset_;
            if (a < -1) {
                a = source_.length(); // False Positive
            }
            for (auto pair : first_block_->ranks_) { //Subtract Excess
                if (pair.first == source_[0]) e -= pair.second;
                else e += pair.second;
            }
            for (auto pair : second_ranks_) { //Sum Excess
                if (pair.first == source_[0]) e += pair.second;
                else e -= pair.second;
            }

        } else {
            for (auto pair : first_block_->ranks_) { //Subtract Excess
                if (pair.first == source_[0]) e -= pair.second;
                else e += pair.second;
            }
            for (auto pair : second_ranks_) { //Sum Excess
                if (pair.first == source_[0]) e += pair.second;
                else e -= pair.second;
            }
        }
    }


    return a;
}





int PBackBlock::min_excess(int i, int j, int& e) {
    if (i == 0 && (j == length()-1 || j == source_.length() - start_index_ - 1)) {
        for (auto pair : ranks_) {
            if (pair.first == source_[0]) e += pair.second;
            else e -= pair.second;
        }
        return min_prefix_excess_;
    }



    int m = 2;
    if (i + offset_ >= first_block_->length()) {
        m = second_block_->min_excess(offset_ + i - first_block_->length(), offset_ + j - first_block_->length(), e);
    } else if (j + offset_ < first_block_->length()) {
        m = first_block_->min_excess(offset_ + i, offset_ + j, e);
    } else {
        int excess = 0;

        if (i == 0) {
            m = prefix_first_block_min_excess_;
            for (auto pair : second_ranks_) {
                if (pair.first == source_[0]) excess += pair.second;
                else excess -= pair.second;
            }
        } else {
            m = first_block_->min_excess(offset_ + i, first_block_->length()-1, excess);
        }


        if (j == length()-1 || j == source_.length() - start_index_ - 1) {
            m = (m < prefix_second_block_min_excess_ + excess) ? m : prefix_second_block_min_excess_ + excess;
            for (auto pair : ranks_) {
                if (pair.first == source_[0]) excess += pair.second;
                else excess -= pair.second;
            }
            for (auto pair : second_ranks_) {
                if (pair.first == source_[0]) excess -= pair.second;
                else excess += pair.second;
            }
        } else {
            int be = excess;
            int min = second_block_->min_excess(0, offset_ + j - first_block_->length(), excess);
            m = (m < min + be) ? m : min + be;
        }
        e += excess;
    }

    return m;
}

int PBackBlock::min_select(int i, int j, int& t, int& e, int m) {
    int a = -1;
    if (i + offset_ >= first_block_->length()) {
        a = second_block_->min_select(offset_ + i - first_block_->length(), offset_ + j - first_block_->length(), t, e, m);
        if (a != -1) a += first_block_->length() - offset_;
    } else if (j + offset_ < first_block_->length()) {
        a = first_block_->min_select(offset_ + i, offset_ + j, t, e, m);
        if (a != -1) a -= offset_;
    } else {
        int excess = 0;

        if (i == 0) {
            if (m < prefix_first_block_min_excess_) {
                for (auto pair : second_ranks_) {
                    if (pair.first == source_[0]) excess += pair.second;
                    else excess -= pair.second;
                }
            }
            else if (m == prefix_first_block_min_excess_) {
                if (t > prefix_min_first_block_count_) {
                    t -= prefix_min_first_block_count_;
                    for (auto pair : second_ranks_) {
                        if (pair.first == source_[0]) excess += pair.second;
                        else excess -= pair.second;
                    }
                } else {
                    a = first_block_->min_select(offset_ + i, first_block_->length()-1, t, excess, m);
                    if (a != -1) a -= offset_;
                }
            }
        } else {
            a = first_block_->min_select(offset_ + i, first_block_->length()-1, t, excess, m);
            if (a != -1) a -= offset_;
        }

        if (a == -1) {
            if (j == length() - 1 || j == source_.length() - start_index_ - 1) {
                if (m < excess + prefix_second_block_min_excess_) {
                    for (auto pair : ranks_) {
                        if (pair.first == source_[0]) excess += pair.second;
                        else excess -= pair.second;
                    }
                    for (auto pair : second_ranks_) {
                        if (pair.first == source_[0]) excess -= pair.second;
                        else excess += pair.second;
                    }
                }
                else if (m == excess + prefix_second_block_min_excess_) {
                    if (t > prefix_min_second_block_count_) {
                        t -= prefix_min_second_block_count_;
                        for (auto pair : ranks_) {
                            if (pair.first == source_[0]) excess += pair.second;
                            else excess -= pair.second;
                        }
                        for (auto pair : second_ranks_) {
                            if (pair.first == source_[0]) excess -= pair.second;
                            else excess += pair.second;
                        }
                    } else {
                        a = second_block_->min_select(0, offset_ + j - first_block_->length(), t, excess, m-excess);
                        if (a != -1) a += first_block_->length() - offset_;
                    }
                }

            } else {
                a = second_block_->min_select(0, offset_ + j - first_block_->length(), t, excess, m-excess);
                if (a != -1) a += first_block_->length() - offset_;
            }
        }
        e += excess;
    }

    return a;
}

int PBackBlock::min_count(int i, int j, int& e, int & m) {
    if (i == 0 && (j == length()-1 || j == source_.length() - start_index_ - 1)) {
        for (auto pair : ranks_) {
            if (pair.first == source_[0]) e += pair.second;
            else e -= pair.second;
        }
        m = min_prefix_excess_;
        return prefix_min_count_;
    }



    m = 2;
    int count = 0;
    if (i + offset_ >= first_block_->length()) {
        count = second_block_->min_count(offset_ + i - first_block_->length(), offset_ + j - first_block_->length(), e, m);
    } else if (j + offset_ < first_block_->length()) {
        count = first_block_->min_count(offset_ + i, offset_ + j, e, m);
    } else {
        int excess = 0;

        if (i == 0) {
            m = prefix_first_block_min_excess_;
            count = prefix_min_first_block_count_;
            for (auto pair : second_ranks_) {
                if (pair.first == source_[0]) excess += pair.second;
                else excess -= pair.second;
            }
        } else {
            count = first_block_->min_count(offset_ + i, first_block_->length()-1, excess, m);
        }


        if (j == length()-1 || j == source_.length() - start_index_ - 1) {
            if (m == prefix_second_block_min_excess_ + excess) count += prefix_min_second_block_count_;
            if (m > prefix_second_block_min_excess_ + excess) count = prefix_min_second_block_count_;
            m = (m < prefix_second_block_min_excess_ + excess) ? m : prefix_second_block_min_excess_ + excess;

            for (auto pair : ranks_) {
                if (pair.first == source_[0]) excess += pair.second;
                else excess -= pair.second;
            }
            for (auto pair : second_ranks_) {
                if (pair.first == source_[0]) excess -= pair.second;
                else excess += pair.second;
            }
        } else {
            int be = excess;
            int min = -1;
            int count2 = second_block_->min_count(0, offset_ + j - first_block_->length(), excess, min);
            if (m == min + be) count += count2;
            if (m > min + be) count = count2;
            m = (m < min + be) ? m : min + be;
        }
        e += excess;
    }

    return count;
}


int PBackBlock::max_excess(int i, int j, int& e) {
    if (i == 0 && (j == length()-1 || j == source_.length() - start_index_ - 1)) {
        for (auto pair : ranks_) {
            if (pair.first == source_[0]) e += pair.second;
            else e -= pair.second;
        }
        return max_prefix_excess_;
    }



    int M = -1;
    if (i + offset_ >= first_block_->length()) {
        M = second_block_->max_excess(offset_ + i - first_block_->length(), offset_ + j - first_block_->length(), e);
    } else if (j + offset_ < first_block_->length()) {
        M = first_block_->max_excess(offset_ + i, offset_ + j, e);
    } else {
        int excess = 0;

        if (i == 0) {
            M = prefix_first_block_max_excess_;
            for (auto pair : second_ranks_) {
                if (pair.first == source_[0]) excess += pair.second;
                else excess -= pair.second;
            }
        } else {
            M = first_block_->max_excess(offset_ + i, first_block_->length()-1, excess);
        }


        if (j == length()-1 || j == source_.length() - start_index_ - 1) {
            M = (M > prefix_second_block_max_excess_ + excess) ? M : prefix_second_block_max_excess_ + excess;
            for (auto pair : ranks_) {
                if (pair.first == source_[0]) excess += pair.second;
                else excess -= pair.second;
            }
            for (auto pair : second_ranks_) {
                if (pair.first == source_[0]) excess -= pair.second;
                else excess += pair.second;
            }
        } else {
            int be = excess;
            int max = second_block_->max_excess(0, offset_ + j - first_block_->length(), excess);
            M = (M > max + be) ? M : max + be;
        }
        e += excess;
    }

    return M;
}


int PBackBlock::positive_fwdsearch(int i, int d, int& e) {
    int a = -1;
    if (i+1 + offset_ < first_block_->length()) {
        a = first_block_->positive_fwdsearch(i+offset_, d , e);
        if (a != -1) {
            a -= offset_;
            for (auto pair : ranks_) { //Sum Excess
                if (pair.first == source_[0]) e += pair.second;
                else e -= pair.second;
            }
            for (auto pair : second_ranks_) { //Subtract Excess
                if (pair.first == source_[0]) e -= pair.second;
                else e += pair.second;
            }
        } else {
            if (second_block_ != nullptr){ // Be careful on no padding version
                if (e + prefix_second_block_max_excess_ >= d) {
                    a = second_block_->positive_fwdsearch(-1, d, e);
                    a += first_block_->length() - offset_;
                    for (auto pair : ranks_) { //Sum Excess
                        if (pair.first == source_[0]) e += pair.second;
                        else e -= pair.second;
                    }
                    for (auto pair : second_ranks_) { //Subtract Excess
                        if (pair.first == source_[0]) e -= pair.second;
                        else e += pair.second;
                    }

                    for (auto pair : second_block_->ranks_) { //Subtract Excess
                        if (pair.first == source_[0]) e -= pair.second;
                        else e += pair.second;
                    }

                } else {
                    for (auto pair : ranks_) { //Sum Excess
                        if (pair.first == source_[0]) e += pair.second;
                        else e -= pair.second;
                    }
                    for (auto pair : second_ranks_) { //Subtract Excess
                        if (pair.first == source_[0]) e -= pair.second;
                        else e += pair.second;
                    }
                }
            }
        }
    }else {
        a = second_block_->positive_fwdsearch(offset_+i-first_block_->length(), d, e);
        if (a != -1) {
            a += first_block_->length() - offset_;
            if (a >= length()) a = -1; // False positive
            for (auto pair : ranks_) { //Sum Excess
                if (pair.first == source_[0]) e += pair.second;
                else e -= pair.second;
            }
            for (auto pair : second_ranks_) { //Subtract Excess
                if (pair.first == source_[0]) e -= pair.second;
                else e += pair.second;
            }

            for (auto pair : second_block_->ranks_) { //Subtract Excess
                if (pair.first == source_[0]) e -= pair.second;
                else e += pair.second;
            }
        } else {
            for (auto pair : ranks_) { //Sum Excess
                if (pair.first == source_[0]) e += pair.second;
                else e -= pair.second;
            }
            for (auto pair : second_ranks_) { //Subtract Excess
                if (pair.first == source_[0]) e -= pair.second;
                else e += pair.second;
            }

            for (auto pair : second_block_->ranks_) { //Subtract Excess
                if (pair.first == source_[0]) e -= pair.second;
                else e += pair.second;
            }
        }
    }

    return a;
}


int PBackBlock::test_fwdsearch(int i, int d, int& e, int& c) {
    ++c;
    int a = -1;
    if (i+1 + offset_ < first_block_->length()) {
        a = first_block_->test_fwdsearch(i+offset_, d , e,c);
        if (a != -1) {
            a -= offset_;
            for (auto pair : ranks_) { //Sum Excess
                if (pair.first == source_[0]) e += pair.second;
                else e -= pair.second;
            }
            for (auto pair : second_ranks_) { //Subtract Excess
                if (pair.first == source_[0]) e -= pair.second;
                else e += pair.second;
            }
        } else {
            if (second_block_ != nullptr){ // Be careful on no padding version
                if (e + prefix_second_block_min_excess_ <= d) {
                    a = second_block_->test_fwdsearch(-1, d, e,c);
                    a += first_block_->length() - offset_;
                    for (auto pair : ranks_) { //Sum Excess
                        if (pair.first == source_[0]) e += pair.second;
                        else e -= pair.second;
                    }
                    for (auto pair : second_ranks_) { //Subtract Excess
                        if (pair.first == source_[0]) e -= pair.second;
                        else e += pair.second;
                    }

                    for (auto pair : second_block_->ranks_) { //Subtract Excess
                        if (pair.first == source_[0]) e -= pair.second;
                        else e += pair.second;
                    }

                } else {
                    for (auto pair : ranks_) { //Sum Excess
                        if (pair.first == source_[0]) e += pair.second;
                        else e -= pair.second;
                    }
                    for (auto pair : second_ranks_) { //Subtract Excess
                        if (pair.first == source_[0]) e -= pair.second;
                        else e += pair.second;
                    }
                }
            }
        }
    }else {
        a = second_block_->test_fwdsearch(offset_+i-first_block_->length(), d, e, c);
        if (a != -1) {
            a += first_block_->length() - offset_;
            if (a >= length()) a = -1; // False positive
            for (auto pair : ranks_) { //Sum Excess
                if (pair.first == source_[0]) e += pair.second;
                else e -= pair.second;
            }
            for (auto pair : second_ranks_) { //Subtract Excess
                if (pair.first == source_[0]) e -= pair.second;
                else e += pair.second;
            }

            for (auto pair : second_block_->ranks_) { //Subtract Excess
                if (pair.first == source_[0]) e -= pair.second;
                else e += pair.second;
            }
        } else {
            for (auto pair : ranks_) { //Sum Excess
                if (pair.first == source_[0]) e += pair.second;
                else e -= pair.second;
            }
            for (auto pair : second_ranks_) { //Subtract Excess
                if (pair.first == source_[0]) e -= pair.second;
                else e += pair.second;
            }

            for (auto pair : second_block_->ranks_) { //Subtract Excess
                if (pair.first == source_[0]) e -= pair.second;
                else e += pair.second;
            }
        }
    }

    return a;
}



int PBackBlock::test_fwdsearch(int i, int j, int d, int& e, int& c) { // Implementing this

    if (i == -1 && (j == length()-1 || j == source_.length() - start_index_ - 1) && e + min_prefix_excess_ > d) {
        for (auto pair : ranks_) {
            if (pair.first == source_[0]) e += pair.second;
            else e -= pair.second;
        }
        return -1;
    }

    ++c;
    int a = -1;

    if (i+1 + offset_ < first_block_->length() && j + offset_ < first_block_->length()) {

        if (i + 1 == 0 && j + offset_ == first_block_->length()-1) {
            if (e + prefix_first_block_min_excess_ <= d) {
                a = first_block_->test_fwdsearch(i+offset_, first_block_->length()-1, d , e,c); // Secure search
                a -= offset_;
            } else {
                for (auto pair : second_ranks_) { //Add Excess
                    if (pair.first == source_[0]) e += pair.second;
                    else e -= pair.second;
                }
            }
        } else {
            a = first_block_->test_fwdsearch(i+offset_, j+offset_, d , e,c);
            if (a != -1) {
                a -= offset_;
            }
        }


    } else if (i+1 + offset_ >= first_block_->length() && j + offset_ >= first_block_->length()) {
        if (i+1 + offset_ == first_block_->length() && j == length()-1) {
            if (e + prefix_second_block_min_excess_ <= d) {
                a = second_block_->test_fwdsearch(-1, offset_+j-first_block_->length(), d, e,c); // Secure search
                a += first_block_->length() - offset_;
            } else {
                for (auto pair : ranks_) { //Sum Excess
                    if (pair.first == source_[0]) e += pair.second;
                    else e -= pair.second;
                }
                for (auto pair : second_ranks_) { //Subtract Excess
                    if (pair.first == source_[0]) e -= pair.second;
                    else e += pair.second;
                }
            }
        } else {
            /*
            //NUEVO
            if (e + prefix_second_block_min_excess_ > d) {
                e += 2*second_block_->test_rank(source_[0], offset_ + j - first_block_->length(), c) - (offset_ + j - first_block_->length())- 1;
            }
            //NUEVO
            else {
             */
            a = second_block_->test_fwdsearch(offset_+i-first_block_->length(), offset_+j-first_block_->length(), d, e, c);
            if (a != -1) {
                a += first_block_->length() - offset_;
            }
            //}
        }




    } else {
        if (i + 1 == 0) {
            if (e + prefix_first_block_min_excess_ <= d) {
                a = first_block_->test_fwdsearch(i+offset_, first_block_->length()-1, d , e,c); // Secure search
                a -= offset_;
            } else {
                for (auto pair : second_ranks_) { //Add Excess
                    if (pair.first == source_[0]) e += pair.second;
                    else e -= pair.second;
                }
            }
        } else {
            a = first_block_->test_fwdsearch(i+offset_, first_block_->length()-1, d , e,c); // Secure search
            if (a != -1) a -= offset_;
        }

        if (a == -1) {
            if (j == length()-1) {
                if (e + prefix_second_block_min_excess_ <= d) {
                    a = second_block_->test_fwdsearch(-1, offset_+j-first_block_->length(), d, e,c); // Secure search
                    a += first_block_->length() - offset_;
                } else {
                    for (auto pair : ranks_) { //Sum Excess
                        if (pair.first == source_[0]) e += pair.second;
                        else e -= pair.second;
                    }
                    for (auto pair : second_ranks_) { //Subtract Excess
                        if (pair.first == source_[0]) e -= pair.second;
                        else e += pair.second;
                    }
                }
            } else {
                /*
                //NUEVO
                if (e + prefix_second_block_min_excess_ > d) {
                    e += 2*second_block_->test_rank(source_[0], offset_ + j - first_block_->length(), c) - (offset_ + j - first_block_->length())- 1;
                }
                //NUEVO
                else {
                 */
                    a = second_block_->test_fwdsearch(-1, offset_ + j - first_block_->length(), d, e, c);
                    if (a != -1) a += first_block_->length() - offset_;
                //}
            }
        }
    }

    return a;
}


int PBackBlock::test_bwdsearch(int i, int j, int d, int& e, int& c) { // Let's supposse i <= j
    ++c;
    int a = source_.length();
    if (i + offset_ < first_block_->length() && j + offset_ < first_block_->length()) {
        a = first_block_->test_bwdsearch(i+offset_, j+offset_, d , e,c);
        if (a != source_.length()) {
            a -= offset_;
        }
    } else if (i + offset_ >= first_block_->length() && j + offset_ >= first_block_->length()) {
        a = second_block_->test_bwdsearch(offset_+i-first_block_->length(), offset_+j-first_block_->length(), d, e, c);
        if (a != source_.length()) {
            a += first_block_->length() - offset_;
        }
    } else {
        if (j == length() - 1) {
            int second_block_excess = 0;
            for (auto pair : ranks_) { //Add Excess
                if (pair.first == source_[0]) second_block_excess += pair.second;
                else second_block_excess -= pair.second;
            }
            for (auto pair : second_ranks_) { //Subtract Excess
                if (pair.first == source_[0]) second_block_excess -= pair.second;
                else second_block_excess += pair.second;
            }

            int reached = ((second_block_excess - prefix_second_block_min_excess_) > second_block_excess) ? (
                    second_block_excess - prefix_second_block_min_excess_) : second_block_excess;
            if (e + reached >= -d) {
                a = second_block_->test_bwdsearch(0, offset_ + j - first_block_->length(), d, e, c); // Secure search
                a += first_block_->length() - offset_;
            } else {
                e += second_block_excess;
            }
        } else {
            a = second_block_->test_bwdsearch(0, offset_ + j - first_block_->length(), d, e, c); // Secure search
            if (a != source_.length()) a += first_block_->length() - offset_;
        }

        if (a == source_.length()) {
            if (i == 0) {
                int first_block_excess = 0;
                for (auto pair : second_ranks_) { //Add Excess
                    if (pair.first == source_[0]) first_block_excess += pair.second;
                    else first_block_excess -= pair.second;
                }

                int reached = ((first_block_excess - prefix_first_block_min_excess_) > first_block_excess) ? (
                        first_block_excess - prefix_first_block_min_excess_) : first_block_excess;
                if (e + reached >= -d) {
                    a = first_block_->test_bwdsearch(i + offset_, first_block_->length()-1, d, e, c); // Secure search
                    a -= offset_;
                } else {
                    e += first_block_excess;
                }
            } else {
                a = first_block_->test_bwdsearch(i + offset_, first_block_->length()-1,d, e, c); // Secure search
                if (a != source_.length()) a -= offset_;
            }
        }
    }
    return a;
}


int PBackBlock::test_bwdsearch(int i, int d, int& e, int& c) {
    ++c;
    int a = source_.length();
    if (i + offset_ >= first_block_->length()) {
        a = second_block_->test_bwdsearch(offset_ + i - first_block_->length(), d, e,c);
        if (a != source_.length()) {
            a += first_block_->length() - offset_;
            for (auto pair : second_ranks_) { //Sum Excess
                if (pair.first == source_[0]) e += pair.second;
                else e -= pair.second;
            }
        } else {
            int excess = 0;
            for (auto pair : second_ranks_) { //Sum Excess
                if (pair.first == source_[0]) excess += pair.second;
                else excess -= pair.second;
            }
            int reached = ((excess - prefix_first_block_min_excess_) > excess) ? (excess - prefix_first_block_min_excess_) : excess;
            if (e + reached >= -d) {
                a = first_block_->test_bwdsearch(first_block_->length() - 1, d, e,c); //Secure search
                a -= offset_;
                e += excess;
                for (auto pair : first_block_->ranks_) { //Subtract Excess
                    if (pair.first == source_[0]) e -= pair.second;
                    else e += pair.second;
                }
            } else {
                e += excess;
            }
        }
    } else {
        a = first_block_->test_bwdsearch(i + offset_, d, e,c);
        if (a != source_.length()) {
            a -= offset_;
            if (a < -1) {
                a = source_.length(); // False Positive
            }
            for (auto pair : first_block_->ranks_) { //Subtract Excess
                if (pair.first == source_[0]) e -= pair.second;
                else e += pair.second;
            }
            for (auto pair : second_ranks_) { //Sum Excess
                if (pair.first == source_[0]) e += pair.second;
                else e -= pair.second;
            }

        } else {
            for (auto pair : first_block_->ranks_) { //Subtract Excess
                if (pair.first == source_[0]) e -= pair.second;
                else e += pair.second;
            }
            for (auto pair : second_ranks_) { //Sum Excess
                if (pair.first == source_[0]) e += pair.second;
                else e -= pair.second;
            }
        }
    }


    return a;
}


int PBackBlock::test_min_excess(int i, int j, int& e, int& c) {


    if (i == 0 && (j == length()-1 || j == source_.length() - start_index_ - 1)) {
        for (auto pair : ranks_) {
            if (pair.first == source_[0]) e += pair.second;
            else e -= pair.second;
        }
        return min_prefix_excess_;
    }

    ++c;


    int m = 2;
    if (i + offset_ >= first_block_->length()) {
        m = second_block_->test_min_excess(offset_ + i - first_block_->length(), offset_ + j - first_block_->length(), e,c);
    } else if (j + offset_ < first_block_->length()) {
        m = first_block_->test_min_excess(offset_ + i, offset_ + j, e,c);
    } else {
        int excess = 0;

        if (i == 0) {
            m = prefix_first_block_min_excess_;
            for (auto pair : second_ranks_) {
                if (pair.first == source_[0]) excess += pair.second;
                else excess -= pair.second;
            }
        } else {
            m = first_block_->test_min_excess(offset_ + i, first_block_->length()-1, excess,c);
        }


        if (j == length()-1 || j == source_.length() - start_index_ - 1) {
            m = (m < prefix_second_block_min_excess_ + excess) ? m : prefix_second_block_min_excess_ + excess;
            for (auto pair : ranks_) {
                if (pair.first == source_[0]) excess += pair.second;
                else excess -= pair.second;
            }
            for (auto pair : second_ranks_) {
                if (pair.first == source_[0]) excess -= pair.second;
                else excess += pair.second;
            }
        } else {
            int be = excess;
            int min = second_block_->test_min_excess(0, offset_ + j - first_block_->length(), excess,c);
            m = (m < min + be) ? m : min + be;
        }
        e += excess;
    }

    return m;
}

void PBackBlock::put_fix_references() {
    fix_reference_ = first_block_->start_index_;
}