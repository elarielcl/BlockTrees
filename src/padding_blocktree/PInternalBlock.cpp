//
// Created by sking32 on 5/18/18.
//

#include "padding_blocktree/PInternalBlock.h"

#include <iostream>

#include "padding_blocktree/PLeafBlock.h"
#include "padding_blocktree/PBackBlock.h"

PInternalBlock::PInternalBlock(PBlock* parent, int64_t start_index, int64_t end_index, int r, int leaf_length, std::string& source, int child_number):
        PBlock(parent, start_index, end_index, r, leaf_length, source, child_number) {
    int next_length = length()/r_;
    if (next_length <= leaf_length_) {
        for (int i = 0; i < r_; ++i) {
            int init = start_index_ + i * next_length;
            int end = start_index_ + (i + 1) * next_length - 1;
            if (init < source_.size()) {
                PBlock *child = new PLeafBlock(this, init, end, r_, leaf_length_, source_, i);
                children_.push_back(child);
            }
        }
    } else {
        for (int i = 0; i < r_; ++i) {
            int init = start_index_ + i * next_length;
            int end = start_index_ + (i + 1) * next_length - 1;
            if (init < source_.size()) {
                PBlock *child = new PInternalBlock(this, init, end, r_, leaf_length_, source_, i);
                children_.push_back(child);
            }
        }
    }
}

PInternalBlock::~PInternalBlock() {
    for (int i = children_.size()-1 ; i>=0; i--)
        delete children_[i];
}

int PInternalBlock::add_rank_select_support(int c) {
    int r = 0;
    for (PBlock* child: children_) {
        child->prefix_ranks_[c] = r;
        r += child->add_rank_select_support(c);
        child->cumulated_ranks_[c] = r;
    }
    ranks_[c] = r;
    return ranks_[c];
}

int PInternalBlock::test_rank(int c, int i, int& counter) {
    ++counter;
    int cumulative_length = 0;
    for (PBlock* child: children_) {
        cumulative_length += child->length();
        if (i < cumulative_length) return child->prefix_ranks_[c] + child->test_rank(c, i-(cumulative_length-child->length()), counter);
    }
    return 0;
}

int PInternalBlock::rank(int c, int i) {
    int cumulative_length = 0;
    for (PBlock* child: children_) {
        cumulative_length += child->length();
        if (i < cumulative_length) return child->prefix_ranks_[c] + child->rank(c, i-(cumulative_length-child->length()));
    }
    return 0;
}

int PInternalBlock::rank_alternative(int c, int i) {
    int cumulative_length = 0;
    int r = 0;
    for (PBlock* child: children_) {
        cumulative_length += child->length();
        if (i < cumulative_length) return r + child->rank_alternative(c, i-(cumulative_length-child->length()));
        r += child->ranks_[c];
    }
    return 0;
}

int PInternalBlock::better_rank(int c, int i) {
    int cumulative_length = 0;
    for (PBlock* child: children_) {
        cumulative_length += child->length();
        if (i < cumulative_length) return ((child->child_number_ == 0)? 0 : child->parent_->children_[child->child_number_-1]->cumulated_ranks_[c]) + child->better_rank(c, i-(cumulative_length-child->length()));
    }
    return 0;
}

int PInternalBlock::select(int c, int j) {
    int cumulative_length = 0;
    int r = 0;
    for (auto it = children_.begin(); it != children_.end(); ++it) {
        if ((it+1) == children_.end()) return (*it)->select(c, j-r) + cumulative_length;
        if ((*(it+1))->prefix_ranks_[c] >= j) return (*it)->select(c, j-r) + cumulative_length;
        r = (*(it+1))->prefix_ranks_[c];
        cumulative_length += (*it)->length();
    }
    return -1;
}


int PInternalBlock::select_alternative(int c, int j) {
    int cumulative_length = 0;
    int r = 0;
    for (auto it = children_.begin(); it != children_.end(); ++it) {
        if ((it+1) == children_.end()) return (*it)->select_alternative(c, j-r) + cumulative_length;
        if (r + (*it)->ranks_[c] >= j) return (*it)->select_alternative(c, j-r) + cumulative_length;
        r += (*it)->ranks_[c];
        cumulative_length += (*it)->length();
    }
    return -1;
}

int PInternalBlock::better_select(int c, int j) {
    int cumulative_length = 0;
    int r = 0;
    for (PBlock* child: children_) {
        if (j <= child->cumulated_ranks_[c]) return cumulative_length + child->better_select(c, j-r);
        cumulative_length += child->length();
        r = child->cumulated_ranks_[c];
    }
    return -1;
}

void PInternalBlock::print() {
    std::cout << " (" << start_index_ << "," << end_index_ << ") " ;
}

std::vector<PBlock*>& PInternalBlock::children() {
    return children_;
}

void PInternalBlock::put_child(int i, PBlock* b) {
    PBlock* old_child = children_[i];
    delete old_child;
    children_[i] = b;
}


int PInternalBlock::number_of_nodes() {
    int c = 1;
    for (PBlock* child: children_)
        c += child->number_of_nodes();
    return c;
}

int PInternalBlock::number_of_back_blocks(){
    int c = 0;
    for (PBlock* child: children_)
        c += child->number_of_back_blocks();
    return c;
}

int PInternalBlock::number_of_internal_blocks() {
    int c = 1;
    for (PBlock* child: children_)
        c += child->number_of_internal_blocks();
    return c;
}

int PInternalBlock::number_of_leaf_blocks() {
    int c = 0;
    for (PBlock* child: children_)
        c += child->number_of_leaf_blocks();
    return c;
}

int PInternalBlock::height() {
    return 1 + children_[0]->height();
}

int PInternalBlock::number_of_leaves() {
    int c = 0;
    for (PBlock* child: children_)
        c += child->number_of_leaves();
    return c;
}

void PInternalBlock::clean_unnecessary_expansions() {
    for (std::vector<PBlock *>::reverse_iterator rit = children_.rbegin(); rit != children_.rend(); ++rit) {
        PBlock *b = (*rit);
        b->clean_unnecessary_expansions();
    }

    bool all_children_leaves = true;
    for (PBlock* child : children_)
        all_children_leaves = all_children_leaves && child->is_leaf();

    if (all_children_leaves && pointing_to_me_ == 0 && first_block_->start_index_ < start_index_ && second_block_!=this) {
        PBackBlock* bb = new PBackBlock(parent_, start_index_, end_index_, r_, leaf_length_, source_,
                                      child_number_, first_block_, second_block_, offset_);
        bb->level_index_ = level_index_;
        bb->first_occurrence_level_index_ = first_occurrence_level_index_;
        bb->left_ = true;
        bb->right_ = true;
        parent_->put_child(child_number_, bb);
    } else { //To avoid dangling references
        first_block_ = this;
        second_block_ = nullptr;
    }

}

int PInternalBlock::clean_unnecessary_expansions(int c) {
    int pointing_to_subtree = 0;
    for (std::vector<PBlock *>::reverse_iterator rit = children_.rbegin(); rit != children_.rend(); ++rit) {
        PBlock *b = (*rit);
        pointing_to_subtree += b->clean_unnecessary_expansions(c);
    }
    if (pointing_to_subtree == 0 && color_ < c && first_block_->start_index_ < start_index_ && second_block_!=this) {
        PBackBlock* bb = new PBackBlock(parent_, start_index_, end_index_, r_, leaf_length_, source_,
                                      child_number_, first_block_, second_block_, offset_);
        bb->level_index_ = level_index_;
        bb->first_occurrence_level_index_ = first_occurrence_level_index_;
        bb->left_ = true;
        bb->right_ = true;
        bb->color_ = color_;
        bb->fix_reference_ = bb->first_block_->start_index_;
        if (color_+1 > first_block_->color_) first_block_->color_ = color_+1;
        if (second_block_ != nullptr && color_+1 > second_block_->color_) {
            second_block_->color_ = color_+1;
        }
        parent_->put_child(child_number_, bb);
    } else { //To avoid dangling references
        first_block_ = this;
        second_block_ = nullptr;
    }
    pointing_to_subtree += pointing_to_me_;
    return pointing_to_subtree;
}

bool PInternalBlock::is_leaf() {
    return false;
}

int PInternalBlock::access(int i) {
    int cumulative_length = 0;
    for (PBlock* child: children_) {
        cumulative_length += child->length();
        if (i < cumulative_length) return child->access(i-(cumulative_length-child->length()));
    }
    return -1;
}

int PInternalBlock::access_2(int i, int&a) {
    int cumulative_length = 0;
    for (PBlock* child: children_) {
        cumulative_length += child->length();
        if (i < cumulative_length) return child->access_2(i-(cumulative_length-child->length()), a);
    }
    return -1;
}

int PInternalBlock::number_of_selfreferences() {
    int n = 0;
    for (PBlock* child: children_)
        n += child->number_of_selfreferences();
    return n;
}

bool PInternalBlock::check_heuristic() {
    bool check = true;
    for (PBlock* child : children_)
        check = check && child->check_heuristic();
    return check && this == first_block_;
}

bool PInternalBlock::check() {
    bool check = true;
    for (PBlock* child : children_)
        check = check && child->check();
    return check && first_block_->start_index_ <= start_index_;
}

bool PInternalBlock::contains_double_pointer(bool from_back) {
    if (from_back) return false;
    for (PBlock* child : children_)
        if (child->contains_double_pointer(false)) return true;
    return false;
}

int PInternalBlock::compute_max_hop() {
    int max_hop = 0;
    for (PBlock* child: children_) {
        int child_max_hop = child->compute_max_hop();
        max_hop = (max_hop > child_max_hop) ? max_hop : child_max_hop;
    }
    return max_hop;
}

void PInternalBlock::put_fix_references() {
    for (PBlock* b: children_) {
        b->put_fix_references();
    }
}