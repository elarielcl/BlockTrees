//
// Created by sking32 on 3/9/18.
//

#include "paper_blocktree/InternalBlock.h"

#include <iostream>

#include "paper_blocktree/LeafBlock.h"
#include "paper_blocktree/BackBlock.h"

InternalBlock::InternalBlock(Block* parent, int start_index, int end_index, int r, int max_leaf_length, std::string& source, int child_number, int max_length_level_block):
        Block(parent, start_index, end_index, r, max_leaf_length, source, child_number) {
    int next_max_length_level_block = (max_length_level_block % r_) ? max_length_level_block/r_ + 1 : max_length_level_block/r_;
    bool children_leaves = next_max_length_level_block <= max_leaf_length_ || next_max_length_level_block < r_;
    if (children_leaves) {
        int number_of_big_blocks = (end_index_ - start_index_ + 1) % r_;
        int hop = (end_index_ - start_index_ + 1) / r_ + 1;
        int i;
        for (i = 0; i < number_of_big_blocks; ++i) {
            int init = start_index_ + i * hop;
            int end = start_index_ + (i + 1) * hop - 1;
            Block *child = new LeafBlock(this, init, end, r_, max_leaf_length_, source_, i);
            children_.push_back(child);
        }
        int start_of_small_blocks = start_index_ + number_of_big_blocks * hop;
        --hop;

        for (int j = 0; j < r_ - number_of_big_blocks; ++j) {
            int init = start_of_small_blocks + j * hop;
            int end = start_of_small_blocks + (j + 1) * hop - 1;
            Block *child = new LeafBlock(this, init, end, r_, max_leaf_length_, source_, j + i);
            children_.push_back(child);
        }
    } else {
        int number_of_big_blocks = (end_index_ - start_index_ + 1) % r_;
        int hop = (end_index_ - start_index_ + 1) / r_ + 1;
        int i;
        for (i = 0; i < number_of_big_blocks; ++i) {
            int init = start_index_ + i * hop;
            int end = start_index_ + (i + 1) * hop - 1;
            Block *child = new InternalBlock(this, init, end, r_, max_leaf_length_, source_, i, next_max_length_level_block);
            children_.push_back(child);
        }
        int start_of_small_blocks = start_index_ + number_of_big_blocks * hop;
        --hop;

        for (int j = 0; j < r_ - number_of_big_blocks; ++j) {
            int init = start_of_small_blocks + j * hop;
            int end = start_of_small_blocks + (j + 1) * hop - 1;
            Block *child = new InternalBlock(this, init, end, r_, max_leaf_length_, source_, j + i, next_max_length_level_block);
            children_.push_back(child);
        }
    }
}

InternalBlock::~InternalBlock() {
    for (int i = children_.size()-1 ; i>=0; i--)
        delete children_[i];
}


int InternalBlock::add_rank_select_leaf_support() {
    starts_with_end_leaf_ = source_[start_index_] != source_[0] && source_[start_index_-1] == source_[0];
    for (Block* child: children_) {
        child->prefix_leaf_rank_ = leaf_rank_;
        leaf_rank_ += child->add_rank_select_leaf_support();
        child->cumulated_leaf_rank_ = leaf_rank_;
    }
    return leaf_rank_;
}

int InternalBlock::add_rank_select_support(int c) {
    int r = 0;
    for (Block* child: children_) {
        child->prefix_ranks_[c] = r;
        r += child->add_rank_select_support(c);
        child->cumulated_ranks_[c] = r;
    }
    ranks_[c] = r;
    return ranks_[c];
}

int InternalBlock::rank(int c, int i) {
    int cumulative_length = 0;
    for (Block* child: children_) {
        cumulative_length += child->length();
        if (i < cumulative_length) return child->prefix_ranks_[c] + child->rank(c, i-(cumulative_length-child->length()));
    }
    return 0;
}

int InternalBlock::rank_alternative(int c, int i) {
    int cumulative_length = 0;
    int r = 0;
    for (Block* child: children_) {
        cumulative_length += child->length();
        if (i < cumulative_length) return r + child->rank_alternative(c, i-(cumulative_length-child->length()));
        r += child->ranks_[c];
    }
    return 0;
}

int InternalBlock::better_rank(int c, int i) {
    int cumulative_length = 0;
    for (Block* child: children_) {
        cumulative_length += child->length();
        if (i < cumulative_length) return ((child->child_number_ == 0)? 0 : child->parent_->children_[child->child_number_-1]->cumulated_ranks_[c]) + child->better_rank(c, i-(cumulative_length-child->length()));
    }
    return 0;
}

int InternalBlock::select(int c, int j) {
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


int InternalBlock::select_alternative(int c, int j) {
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

int InternalBlock::better_select(int c, int j) {
    int cumulative_length = 0;
    int r = 0;
    for (Block* child: children_) {
        if (j <= child->cumulated_ranks_[c]) return cumulative_length + child->better_select(c, j-r);
        cumulative_length += child->length();
        r = child->cumulated_ranks_[c];
    }
    return -1;
}

int InternalBlock::leaf_rank(int i) {
    int cumulative_length = 0;
    for (Block* child: children_) {
        cumulative_length += child->length();
        if (i < cumulative_length) return child->prefix_leaf_rank_ + child->leaf_rank(i-(cumulative_length-child->length()));
    }
    return 0;
}

int InternalBlock::leaf_rank_alternative(int i) {
    int cumulative_length = 0;
    int r = 0;
    for (Block* child: children_) {
        cumulative_length += child->length();
        if (i < cumulative_length) return r + child->leaf_rank_alternative(i-(cumulative_length-child->length()));
        r += child->leaf_rank_;
    }
    return 0;
}

int InternalBlock::better_leaf_rank(int i) {
    int cumulative_length = 0;
    for (Block* child: children_) {
        cumulative_length += child->length();
        if (i < cumulative_length) return ((child->child_number_ == 0)? 0 : child->parent_->children_[child->child_number_-1]->cumulated_leaf_rank_) + child->better_leaf_rank(i-(cumulative_length-child->length()));
    }
    return 0;
}

int InternalBlock::leaf_select(int j) {
    int cumulative_length = 0;
    int r = 0;
    for (auto it = children_.begin(); it != children_.end(); ++it) {
        if ((it+1) == children_.end()) return (*it)->leaf_select(j-r) + cumulative_length;
        if ((*(it+1))->prefix_leaf_rank_ >= j) return (*it)->leaf_select(j-r) + cumulative_length;
        r = (*(it+1))->prefix_leaf_rank_;
        cumulative_length += (*it)->length();
    }
    return -1;
}


int InternalBlock::leaf_select_alternative(int j) {
    int cumulative_length = 0;
    int r = 0;
    for (auto it = children_.begin(); it != children_.end(); ++it) {
        if ((it+1) == children_.end()) return (*it)->leaf_select_alternative(j-r) + cumulative_length;
        if (r + (*it)->leaf_rank_ >= j) return (*it)->leaf_select_alternative(j-r) + cumulative_length;
        r += (*it)->leaf_rank_;
        cumulative_length += (*it)->length();
    }
    return -1;
}

int InternalBlock::better_leaf_select(int j) {
    int cumulative_length = 0;
    int r = 0;
    for (Block* child: children_) {
        if (j <= child->cumulated_leaf_rank_) return cumulative_length + child->better_leaf_select(j-r);
        cumulative_length += child->length();
        r = child->cumulated_leaf_rank_;
    }
    return -1;
}



void InternalBlock::print() {
    std::cout << " (" << start_index_ << "," << end_index_ << ") " ;
}

std::vector<Block*>& InternalBlock::children() {
    return children_;
}

void InternalBlock::put_child(int i, Block* b) {
    Block* old_child = children_[i];
    delete old_child;
    children_[i] = b;
}


int InternalBlock::number_of_nodes() {
    int c = 1;
    for (Block* child: children_)
        c += child->number_of_nodes();
    return c;
}

int InternalBlock::number_of_back_blocks(){
    int c = 0;
    for (Block* child: children_)
        c += child->number_of_back_blocks();
    return c;
}

int InternalBlock::number_of_internal_blocks() {
    int c = 1;
    for (Block* child: children_)
        c += child->number_of_internal_blocks();
    return c;
}

int InternalBlock::number_of_leaf_blocks() {
    int c = 0;
    for (Block* child: children_)
        c += child->number_of_leaf_blocks();
    return c;
}

int InternalBlock::height() {
    return 1 + children_[0]->height();
}

int InternalBlock::number_of_leaves() {
    int c = 0;
    for (Block* child: children_)
        c += child->number_of_leaves();
    return c;
}

void InternalBlock::clean_unnecessary_expansions() {
    for (std::vector<Block *>::reverse_iterator rit = children_.rbegin(); rit != children_.rend(); ++rit) {
        Block *b = (*rit);
        b->clean_unnecessary_expansions();
    }

    bool all_children_leaves = true;
    for (Block* child : children_)
        all_children_leaves = all_children_leaves && child->is_leaf();

    if (all_children_leaves && pointing_to_me_ == 0 && first_block_->start_index_ < start_index_ && second_block_!=this) {
        BackBlock* bb = new BackBlock(parent_, start_index_, end_index_, r_, max_leaf_length_, source_,
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



int InternalBlock::clean_unnecessary_expansions(int c) {
    int pointing_to_subtree = 0;
    for (std::vector<Block *>::reverse_iterator rit = children_.rbegin(); rit != children_.rend(); ++rit) {
        Block *b = (*rit);
        pointing_to_subtree += b->clean_unnecessary_expansions(c);
    }
    if (pointing_to_subtree == 0 && color_ < c && first_block_->start_index_ < start_index_ && second_block_!=this) {
        BackBlock* bb = new BackBlock(parent_, start_index_, end_index_, r_, max_leaf_length_, source_,
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

bool InternalBlock::is_leaf() {
    return false;
}

int InternalBlock::access(int i) {
    int cumulative_length = 0;
    for (Block* child: children_) {
        cumulative_length += child->length();
        if (i < cumulative_length) return child->access(i-(cumulative_length-child->length()));
    }
    return -1;
}

int InternalBlock::access_2(int i, int&a) {
    int cumulative_length = 0;
    for (Block* child: children_) {
        cumulative_length += child->length();
        if (i < cumulative_length) return child->access_2(i-(cumulative_length-child->length()), a);
    }
    return -1;
}

int InternalBlock::number_of_selfreferences() {
    int n = 0;
    for (Block* child: children_)
        n += child->number_of_selfreferences();
    return n;
}

bool InternalBlock::check_heuristic() {
    bool check = true;
    for (Block* child : children_)
        check = check && child->check_heuristic();
    return check && this == first_block_;
}

bool InternalBlock::check() {
    bool check = true;
    for (Block* child : children_)
        check = check && child->check();
    return check && first_block_->start_index_ <= start_index_;
}

bool InternalBlock::contains_double_pointer(bool from_back) {
    if (from_back) return false;
    for (Block* child : children_)
        if (child->contains_double_pointer(false)) return true;
    return false;
}

int InternalBlock::compute_max_hop() {
    int max_hop = 0;
    for (Block* child: children_) {
        int child_max_hop = child->compute_max_hop();
        max_hop = (max_hop > child_max_hop) ? max_hop : child_max_hop;
    }
    return max_hop;
}

void InternalBlock::put_fix_references() {
    for (Block* b: children_) {
        b->put_fix_references();
    }
}