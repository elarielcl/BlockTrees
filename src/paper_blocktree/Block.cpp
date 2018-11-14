//
// Created by sking32 on 3/7/18.
//

#include "paper_blocktree/Block.h"

#include <iostream>

Block::Block(Block* parent, int start_index, int end_index, int r, int max_leaf_length,std::string& source, int child_number):
        parent_(parent), start_index_(start_index), end_index_(end_index), r_(r), max_leaf_length_(max_leaf_length), source_(source), child_number_(child_number), left_(false), right_(false), first_block_(
        this), second_block_(nullptr), pointing_to_me_(0), max_hop_(0), color_(0), level_index_(0), first_occurrence_level_index_(0), leaf_rank_(0), starts_with_end_leaf_(false), fix_reference_(-1) {

}

Block::~Block() {

}


int Block::add_rank_select_leaf_support() {
    return 0;
}

int Block::add_rank_select_support(int c) {
    return 0;
}


int Block::leaf_rank(int i) {
    return 0;
}

int Block::leaf_rank_alternative(int i) {
    return 0;
}

int Block::better_leaf_rank(int i) {
    return 0;
}

int Block::rank(int c, int i) {
    return 0;
}

int Block::rank_alternative(int c, int i) {
    return 0;
}

int Block::better_rank(int c, int i) {
    return 0;
}


int Block::leaf_select(int j) {
    return -1;
}

int Block::leaf_select_alternative(int j) {
    return -1;
}

int Block::better_leaf_select(int j) {
    return -1;
}

int Block::select(int c, int j) {
    return -1;
}

int Block::select_alternative(int c, int j) {
    return -1;
}

int Block::better_select(int c, int j) {
    return -1;
}



int Block::length() {
    return end_index_-start_index_+1;
}


void Block::print() {
    std::cout << " (" << start_index_ << "," << end_index_ << ") " ;
}

std::vector<Block*>& Block::children() {
    return children_;
}


void Block::put_child(int i, Block* child) {

}

int Block::number_of_nodes() {
    return 1;
}

int Block::height() {
    return 0;
}

int Block::number_of_leaves() {
    return 1;
}

void Block::clean_unnecessary_expansions() {

}

int Block::clean_unnecessary_expansions(int c) {
    return pointing_to_me_;
}

bool Block::is_leaf() {
    return true;
}

int Block::access(int i) {
    return -1;
}

int Block::access_2(int i, int&a) {
    return -1;
}

int Block::number_of_selfreferences() {
    return 0;
}

bool Block::check_heuristic() {
    return true;
}

bool Block::check() {
    return true;
}

bool Block::contains_double_pointer(bool from_back) {
    return false;
}

std::string Block::represented_string() {
    return source_.substr(start_index_, length());
}

int Block::number_of_internal_blocks() {
    return 0;
}

int Block::number_of_back_blocks() {
    return 0;
}

int Block::number_of_leaf_blocks() {
    return 0;
}

int Block::compute_max_hop() {
    return 0;
}

void Block::put_fix_references() {

}