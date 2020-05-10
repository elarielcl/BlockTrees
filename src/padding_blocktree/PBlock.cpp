//
// Created by sking32 on 5/18/18.
//

#include "padding_blocktree/PBlock.h"

#include <iostream>

PBlock::PBlock(PBlock* parent, int64_t start_index, int64_t end_index, int r, int leaf_length,std::string& source, int child_number):
        parent_(parent), start_index_(start_index), end_index_(end_index), r_(r), leaf_length_(leaf_length), source_(source), child_number_(child_number), left_(false), right_(false), first_block_(
        this), second_block_(nullptr), pointing_to_me_(0), max_hop_(0), color_(0), level_index_(0), first_occurrence_level_index_(0), fix_reference_(-1) {

}

PBlock::~PBlock() {

}

int PBlock::add_rank_select_support(int c) {
    return 0;
}

int PBlock::test_rank(int c, int i, int& counter) {
    return 0;
}

int PBlock::rank(int c, int i) {
    return 0;
}

int PBlock::rank_alternative(int c, int i) {
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

int64_t PBlock::length() {
    return end_index_-start_index_+1;
}

void PBlock::print() {
    std::cout << " (" << start_index_ << "," << end_index_ << ") " ;
}

std::vector<PBlock*>& PBlock::children() {
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

bool PBlock::is_leaf() {
    return true;
}

int PBlock::access(int i) {
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

void PBlock::put_fix_references() {

}