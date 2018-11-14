//
// Created by sking32 on 5/18/18.
//

#include "padding_blocktree/PBlock.h"

#include <iostream>

PBlock::PBlock(PBlock* parent, int64_t start_index, int64_t end_index, int r, int leaf_length,std::string& source, int child_number):
        parent_(parent), start_index_(start_index), end_index_(end_index), r_(r), leaf_length_(leaf_length), source_(source), child_number_(child_number), left_(false), right_(false), first_block_(
        this), second_block_(nullptr), pointing_to_me_(0), max_hop_(0), color_(0), level_index_(0), first_occurrence_level_index_(0), leaf_rank_(0), starts_with_end_leaf_(false), fix_reference_(-1), wsource_(waux){

}

PBlock::PBlock(PBlock* parent, int64_t start_index, int64_t end_index, int r, int leaf_length,std::basic_string<int64_t>& source, int child_number):
        parent_(parent), start_index_(start_index), end_index_(end_index), r_(r), leaf_length_(leaf_length), wsource_(source), child_number_(child_number), left_(false), right_(false), first_block_(
        this), second_block_(nullptr), pointing_to_me_(0), max_hop_(0), color_(0), level_index_(0), first_occurrence_level_index_(0), leaf_rank_(0), starts_with_end_leaf_(false), fix_reference_(-1), source_(aux){

}

PBlock::~PBlock() {

}

int PBlock::add_fwdsearch_support() {
    return 0;
}

int PBlock::add_preffix_min_count_fields() {
    return 0;
}

int PBlock::add_max_fields() {
    return 0;
}

int PBlock::excess() {
    int e = 0;
    std::string rep = represented_string();
    for (char c: rep) {
        e += (source_[0] == c) ? 1:-1;
    }
    return e;
}

int PBlock::min_excess() {
    int e = 0;
    int min = 1;
    std::string rep = represented_string();
    for (char c: rep) {
        e += (source_[0] == c) ? 1:-1;
        if (e < min) min  = e;
    }
    return min;
}

int PBlock::max_excess() {
    int e = 0;
    int max = -1;
    std::string rep = represented_string();
    for (char c: rep) {
        e += (source_[0] == c) ? 1:-1;
        if (e > max) max  = e;
    }
    return max;
}

int PBlock::min_count() {
    int e = 0;
    int min = 1;
    int count = 0;
    std::string rep = represented_string();
    for (char c: rep) {
        e += (source_[0] == c) ? 1:-1;
        if (e == min) ++count;
        if (e < min) {
            min  = e;
            count = 1;
        }
    }
    return count;
}

int PBlock::fwdsearch(int i, int d, int& e) {
    return -1;
}

int PBlock::positive_fwdsearch(int i, int d, int& e) {
    return -1;
}


int PBlock::bwdsearch(int i, int d, int& e) {
    return source_.length();
}

int PBlock::positive_bwdsearch(int i, int d, int& e) {
    return source_.length();
}

int PBlock::min_select(int i, int j, int & t, int & e, int m) {
    return -1;
}

int PBlock::min_count(int i, int j, int & e, int & m) {
    return 0;
}

int PBlock::linear_min_select(int i, int j, int t) {
    int m =linear_min_excess(i,j);
    int excess = 0;
    int count = 0;
    for (int k = i; k <= j; ++k) {
        excess += (source_[k+start_index_] == source_[0]) ? 1 : -1;
        if (excess == m) ++count;
        if (count == t) return k;
    }
    return -1;
}

int PBlock::linear_min_count(int i, int j) {
    int excess = 0;
    int min = 1;
    int count = 0;
    for (int k = i; k <= j; ++k) {
        excess += (source_[k+start_index_] == source_[0]) ? 1 : -1;
        if (excess == min) ++count;
        if (excess < min) {
            min = excess;
            count = 1;
        }
    }
    return count;
}

int PBlock::linear_min_excess(int i, int j) {
    int excess = 0;
    int min = 1;
    for (int k = i; k <= j; ++k) {
        excess += (source_[k+start_index_] == source_[0]) ? 1 : -1;
        if (excess < min) min = excess;
    }
    return min;
}

int PBlock::linear_max_excess(int i, int j) {
    int excess = 0;
    int max = -1;
    for (int k = i; k <= j; ++k) {
        excess += (source_[k+start_index_] == source_[0]) ? 1 : -1;
        if (excess > max) max = excess;
    }
    return max;
}

int PBlock::min_excess(int i, int j, int& e) {
    return 1;
}

int PBlock::max_excess(int i, int j, int& e) {
    return -1;
}


int PBlock::add_rank_select_support(int c) {
    return 0;
}

int PBlock::test_rank(int c, int i, int& counter) {
    return 0;
}

int64_t PBlock::differential_access(int i) {
    return -1;
}

int PBlock::rank(int c, int i) {
    return 0;
}

int PBlock::rank_alternative(int c, int i) {
    return 0;
}

int64_t PBlock::differential_access_alternative(int i) {
    return 0;
}

int64_t PBlock::better_differential_access(int i) {
    return 0;
}

int PBlock::better_rank(int c, int i) {
    return 0;
}

int PBlock::select(int c, int j) {
    return -1;
}

int PBlock::select_alternative(int c, int j) {
    return -1;
}

int PBlock::better_select(int c, int j) {
    return -1;
}

int PBlock::add_rank_select_leaf_support() {
    return 0;
}

int64_t PBlock::add_differential_access_support() {
    return 0;
}

int PBlock::leaf_rank(int i) {
    return 0;
}

int PBlock::leaf_rank_alternative(int i) {
    return 0;
}

int PBlock::better_leaf_rank(int i) {
    return 0;
}

int PBlock::leaf_select(int j) {
    return -1;
}

int PBlock::leaf_select_alternative(int j) {
    return -1;
}

int PBlock::better_leaf_select(int j) {
    return -1;
}
int64_t PBlock::length() {
    return end_index_-start_index_+1;
}


void PBlock::print() {
    std::cout << " (" << start_index_ << "," << end_index_ << ") " ;
}

std::vector<PBlock*>& PBlock::children() {
    return children_;
}

std::vector<PBlock*>& PBlock::wchildren() {
    return children_;
}


void PBlock::put_child(int i, PBlock* child) {

}

int PBlock::number_of_nodes() {
    return 1;
}

int PBlock::height() {
    return 0;
}

int PBlock::number_of_leaves() {
    return 1;
}

void PBlock::clean_unnecessary_expansions() {

}

int PBlock::clean_unnecessary_expansions(int c) {
    return pointing_to_me_;
}


void PBlock::wclean_unnecessary_expansions() {

}

int PBlock::wclean_unnecessary_expansions(int c) {
    return pointing_to_me_;
}

bool PBlock::is_leaf() {
    return true;
}

int PBlock::access(int i) {
    return -1;
}

int PBlock::waccess(int i) {
    return -1;
}

int PBlock::access_2(int i, int&a) {
    return -1;
}

int PBlock::number_of_selfreferences() {
    return 0;
}

bool PBlock::check_heuristic() {
    return true;
}

bool PBlock::check() {
    return true;
}

bool PBlock::contains_double_pointer(bool from_back) {
    return false;
}

std::string PBlock::represented_string() {
    return source_.substr(start_index_, length());
}

std::basic_string<int64_t> PBlock::represented_wstring() {
    return wsource_.substr(start_index_, length());
}

int PBlock::number_of_internal_blocks() {
    return 0;
}

int PBlock::number_of_back_blocks() {
    return 0;
}

int PBlock::number_of_leaf_blocks() {
    return 0;
}

int PBlock::compute_max_hop() {
    return 0;
}


int PBlock::test_fwdsearch(int i, int d, int& e, int& c) {
    return -1;
}

int PBlock::test_fwdsearch(int i, int j, int d, int& e, int& c) {
    return -1;
}

int PBlock::test_bwdsearch(int i, int d, int& e, int& c) {
    return -1;
}

int PBlock::test_bwdsearch(int i, int j, int d, int& e, int& c) {
    return -1;
}

int PBlock::test_min_excess(int i, int j, int& e, int& c) {
    return 1;
}

void PBlock::put_fix_references() {

}