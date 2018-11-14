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

PInternalBlock::PInternalBlock(PBlock* parent, int64_t start_index, int64_t end_index, int r, int leaf_length, std::basic_string<int64_t>& source, int child_number):
        PBlock(parent, start_index, end_index, r, leaf_length, source, child_number) {
    int next_length = length()/r_;
    if (next_length <= leaf_length_) {
        for (int i = 0; i < r_; ++i) {
            int init = start_index_ + i * next_length;
            int end = start_index_ + (i + 1) * next_length - 1;
            if (init < wsource_.size()) {
                PBlock *child = new PLeafBlock(this, init, end, r_, leaf_length_, wsource_, i);
                children_.push_back(child);
            }
        }
    } else {
        for (int i = 0; i < r_; ++i) {
            int init = start_index_ + i * next_length;
            int end = start_index_ + (i + 1) * next_length - 1;
            if (init < wsource_.size()) {
                PBlock *child = new PInternalBlock(this, init, end, r_, leaf_length_, wsource_, i);
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

int64_t PInternalBlock::add_differential_access_support() {
    int64_t r = 0;
    for (PBlock* child : children_) {
        child->prefix_sum_ = r;
        r += child->add_differential_access_support();
        child->cumulated_sum_ = r;
    }
    sum_ = r;
    return sum_;
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

int64_t PInternalBlock::differential_access(int i) {
    int cumulative_length = 0;
    for (PBlock* child: children_) {
        cumulative_length += child->length();
        if (i < cumulative_length) return child->prefix_sum_ + child->differential_access(i-(cumulative_length-child->length()));
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

int64_t PInternalBlock::differential_access_alternative(int i) {
    int cumulative_length = 0;
    int64_t r = 0;
    for (PBlock* child: children_) {
        cumulative_length += child->length();
        if (i < cumulative_length) return r + child->differential_access_alternative(i-(cumulative_length-child->length()));
        r += child->sum_;
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

int64_t PInternalBlock::better_differential_access(int i) {
    int cumulative_length = 0;
    for (PBlock* child: children_) {
        cumulative_length += child->length();
        if (i < cumulative_length) return ((child->child_number_ == 0)? 0 : child->parent_->children_[child->child_number_-1]->cumulated_sum_) + child->better_differential_access(i-(cumulative_length-child->length()));
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

std::vector<PBlock*>& PInternalBlock::wchildren() {
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



void PInternalBlock::wclean_unnecessary_expansions() {
    for (std::vector<PBlock *>::reverse_iterator rit = children_.rbegin(); rit != children_.rend(); ++rit) {
        PBlock *b = (*rit);
        b->wclean_unnecessary_expansions();
    }

    bool all_children_leaves = true;
    for (PBlock* child : children_)
        all_children_leaves = all_children_leaves && child->is_leaf();

    if (all_children_leaves && pointing_to_me_ == 0 && first_block_->start_index_ < start_index_ && second_block_!=this) {
        PBackBlock* bb = new PBackBlock(parent_, start_index_, end_index_, r_, leaf_length_, wsource_,
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

int PInternalBlock::wclean_unnecessary_expansions(int c) {
    int pointing_to_subtree = 0;
    for (std::vector<PBlock *>::reverse_iterator rit = children_.rbegin(); rit != children_.rend(); ++rit) {
        PBlock *b = (*rit);
        pointing_to_subtree += b->wclean_unnecessary_expansions(c);
    }
    if (pointing_to_subtree == 0 && color_ < c && first_block_->start_index_ < start_index_ && second_block_!=this) {
        PBackBlock* bb = new PBackBlock(parent_, start_index_, end_index_, r_, leaf_length_, wsource_,
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

int PInternalBlock::waccess(int i) {
    int cumulative_length = 0;
    for (PBlock* child: children_) {
        cumulative_length += child->length();
        if (i < cumulative_length) return child->waccess(i-(cumulative_length-child->length()));
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

int PInternalBlock::add_fwdsearch_support() {
    int min_excess = 1;
    for (PBlock* child : children_) {
        int min_excess_child = child->add_fwdsearch_support();
        int prefix_excess = 0;
        for (auto pair : child->prefix_ranks_) {
            if (pair.first == source_[0]) prefix_excess += pair.second;
            else prefix_excess -= pair.second;
        }
        if (min_excess_child + prefix_excess < min_excess) min_excess = min_excess_child + prefix_excess;
    }
    min_prefix_excess_ = min_excess;
    return min_prefix_excess_;
}


int PInternalBlock::add_preffix_min_count_fields() {
    int min_excess = 1;
    int count = 0;
    for (PBlock* child : children_) {
        int min_excess_child = child->add_preffix_min_count_fields();
        int prefix_excess = 0;
        for (auto pair : child->prefix_ranks_) {
            if (pair.first == source_[0]) prefix_excess += pair.second;
            else prefix_excess -= pair.second;
        }
        if (min_excess_child + prefix_excess == min_excess) {
            count += child->prefix_min_count_;
        }
        if (min_excess_child + prefix_excess < min_excess) {
            count = child->prefix_min_count_;
            min_excess = min_excess_child + prefix_excess;
        }
    }
    min_prefix_excess_ = min_excess;
    prefix_min_count_ = count;
    return min_prefix_excess_;
}


int PInternalBlock::add_max_fields() {
    int max_excess = -1;
    for (PBlock* child : children_) {
        int max_excess_child = child->add_max_fields();
        int prefix_excess = 0;
        for (auto pair : child->prefix_ranks_) {
            if (pair.first == source_[0]) prefix_excess += pair.second;
            else prefix_excess -= pair.second;
        }
        if (max_excess_child + prefix_excess > max_excess) max_excess = max_excess_child + prefix_excess;
    }
    max_prefix_excess_ = max_excess;
    return max_prefix_excess_;
}


int PInternalBlock::fwdsearch(int i, int d, int& e) {
    int a = -1;
    int cumulative_length = 0;
    bool first_search_done = false;
    for (PBlock* child: children_) {
        cumulative_length += child->length();
        if (!first_search_done) {
            if (i + 1 < cumulative_length) {
                a = child->fwdsearch(i - (cumulative_length - child->length()), d, e);
                if (a != -1) a += cumulative_length - child->length();
                first_search_done = true;
            }
        } else {
            if (a == -1 && e + child->min_prefix_excess_ <= d) {
                a = child->fwdsearch(-1, d, e); // Secure seach
                a += cumulative_length - child->length();
            }
            else {
                for (auto pair : child->ranks_) {
                    if (pair.first == source_[0]) e += pair.second;
                    else e -= pair.second;
                }
            }
        }
    }
    return a;
}


int PInternalBlock::positive_fwdsearch(int i, int d, int& e) {
    int a = -1;
    int cumulative_length = 0;
    bool first_search_done = false;
    for (PBlock* child: children_) {
        cumulative_length += child->length();
        if (!first_search_done) {
            if (i + 1 < cumulative_length) {
                a = child->positive_fwdsearch(i - (cumulative_length - child->length()), d, e);
                if (a != -1) a += cumulative_length - child->length();
                first_search_done = true;
            }
        } else {
            if (a == -1 && e + child->max_prefix_excess_ >= d) {
                a = child->positive_fwdsearch(-1, d, e); // Secure seach
                a += cumulative_length - child->length();
            }
            else {
                for (auto pair : child->ranks_) {
                    if (pair.first == source_[0]) e += pair.second;
                    else e -= pair.second;
                }
            }
        }
    }
    return a;
}

int PInternalBlock::bwdsearch(int i, int d, int& e) {
    int a = source_.length();
    int cumulative_length = 0;
    for (PBlock* child: children_) {
        cumulative_length += child->length();
    }
    bool first_search_done = false;
    for (auto rit = children_.rbegin(); rit != children_.rend(); rit++) {
        PBlock* child = (*rit);
        cumulative_length -= child->length();
        if (!first_search_done) {
            if (i >= cumulative_length) {
                a = child->bwdsearch(i - cumulative_length, d, e);
                if (a != source_.length()) a += cumulative_length;
                first_search_done = true;
            }
        } else {
            int child_excess = 0;
            for (auto pair : child->ranks_) {
                if (pair.first == source_[0]) child_excess += pair.second;
                else child_excess -= pair.second;
            }
            int reached = ((child_excess-child->min_prefix_excess_) > child_excess) ? (child_excess-child->min_prefix_excess_) : child_excess;
            if (a == source_.length() && e + reached >= -d) {
                a = child->bwdsearch(child->length()-1, d, e); // Secure seach
                a += cumulative_length;
            }
            else {
                for (auto pair : child->ranks_) {
                    if (pair.first == source_[0]) e += pair.second;
                    else e -= pair.second;
                }
            }
        }
    }
    return a;
}



int PInternalBlock::positive_bwdsearch(int i, int d, int& e) {
    int a = source_.length();
    int cumulative_length = 0;
    for (PBlock* child: children_) {
        cumulative_length += child->length();
    }
    bool first_search_done = false;
    for (auto rit = children_.rbegin(); rit != children_.rend(); rit++) {
        PBlock* child = (*rit);
        cumulative_length -= child->length();
        if (!first_search_done) {
            if (i >= cumulative_length) {
                a = child->positive_bwdsearch(i - cumulative_length, d, e);
                if (a != source_.length()) a += cumulative_length;
                first_search_done = true;
            }
        } else {
            int child_excess = 0;
            for (auto pair : child->ranks_) {
                if (pair.first == source_[0]) child_excess += pair.second;
                else child_excess -= pair.second;
            }
            int reached = ((child_excess-child->max_prefix_excess_) < child_excess) ? (child_excess-child->max_prefix_excess_) : child_excess;
            if (a == source_.length() && e + reached <= -d) {
                a = child->positive_bwdsearch(child->length()-1, d, e); // Secure seach
                a += cumulative_length;
            }
            else {
                for (auto pair : child->ranks_) {
                    if (pair.first == source_[0]) e += pair.second;
                    else e -= pair.second;
                }
            }
        }
    }
    return a;
}



int PInternalBlock::min_excess(int i, int j, int& e) {
    if (i == 0 && (j == length()-1 || j == source_.length() - start_index_ - 1)) {
        for (auto pair : ranks_) {
            if (pair.first == source_[0]) e += pair.second;
            else e -= pair.second;
        }
        return min_prefix_excess_;
    }

    int m = 2;
    int cumulative_length = 0;
    bool first_search_done = false;
    int excess = 0;
    for (PBlock* child : children_) {
        cumulative_length += child->length();
        if (!first_search_done) {
            if (i < cumulative_length) {
                if (j < cumulative_length) {
                    m = child->min_excess(i - (cumulative_length - child->length()),
                                          j - (cumulative_length - child->length()), excess);
                    break;
                }
                // Suffix min_excess
                m = child->min_excess(i - (cumulative_length - child->length()), child->length()-1, excess);
                first_search_done = true;
            }
        } else {
            if (j < cumulative_length) {
                int be = excess;
                // Prefix min_excess
                int min = child->min_excess(0,
                                            j - (cumulative_length - child->length()), excess);
                m = (m < min + be) ? m : min+be;
                break;
            } else {
                m = (m < excess + child->min_prefix_excess_) ? m : excess + child->min_prefix_excess_;
                for (auto pair : child->ranks_) {
                    if (pair.first == source_[0]) excess += pair.second;
                    else excess -= pair.second;
                }
            }
        }
    }
    e += excess;
    return m;
}


int PInternalBlock::min_select(int i, int j, int &t, int &e, int m) {
    int a = -1;
    int cumulative_length = 0;
    bool first_search_done = false;
    int excess = 0;
    for (PBlock* child : children_) {
        cumulative_length += child->length();
        if (!first_search_done) {
            if (i < cumulative_length) {
                if (j < cumulative_length) {
                    a = child->min_select(i - (cumulative_length - child->length()),
                                          j - (cumulative_length - child->length()), t, excess, m);
                    if (a != -1) a += cumulative_length - child->length();
                    break;
                }
                // Suffix min_excess
                a = child->min_select(i - (cumulative_length - child->length()), child->length()-1, t, excess,m);
                if (a != -1) {
                    a += cumulative_length - child->length();
                    break;
                }
                first_search_done = true;
            }
        } else {
            if (j < cumulative_length) {
                // Prefix min_excess
                a= child->min_select(0,
                                     j - (cumulative_length - child->length()), t,excess, m-excess);
                if (a != -1) a += cumulative_length - child->length();
                break;
            } else {
                if (m == excess + child->min_prefix_excess_) {
                    if (t > prefix_min_count_){
                        t -= prefix_min_count_;
                    } else {
                        a = child->min_select(0, child->length()-1, t,excess, child->min_prefix_excess_/*m-excess*/);
                        if (a != -1) a += cumulative_length - child->length();
                        break;
                    }
                }
                for (auto pair : child->ranks_) {
                    if (pair.first == source_[0]) excess += pair.second;
                    else excess -= pair.second;
                }
            }
        }
    }
    e += excess;
    return a;
}


int PInternalBlock::min_count(int i, int j, int& e, int& m) {
    if (i == 0 && (j == length()-1 || j == source_.length() - start_index_ - 1)) {
        for (auto pair : ranks_) {
            if (pair.first == source_[0]) e += pair.second;
            else e -= pair.second;
        }
        m= min_prefix_excess_;
        return prefix_min_count_;
    }

    int count = 0;
    m = 2;
    int cumulative_length = 0;
    bool first_search_done = false;
    int excess = 0;
    for (PBlock* child : children_) {
        cumulative_length += child->length();
        if (!first_search_done) {
            if (i < cumulative_length) {
                if (j < cumulative_length) {
                    count = child->min_count(i - (cumulative_length - child->length()),
                                          j - (cumulative_length - child->length()), excess, m);
                    break;
                }
                // Suffix min_excess
                count = child->min_count(i - (cumulative_length - child->length()), child->length()-1, excess,m);
                first_search_done = true;
            }
        } else {
            if (j < cumulative_length) {
                int be = excess;
                int min = -1;
                // Prefix min_excess
                int count2 = child->min_count(0,
                                            j - (cumulative_length - child->length()), excess, min);
                if (m == min + be) count += count2;
                if (m > min + be) count = count2;
                m = (m < min + be) ? m : min+be;
                break;
            } else {
                if (m == excess + child->min_prefix_excess_) count += child->prefix_min_count_;
                if (m > excess + child->min_prefix_excess_) count = child->prefix_min_count_;
                m = (m < excess + child->min_prefix_excess_) ? m : excess + child->min_prefix_excess_;
                for (auto pair : child->ranks_) {
                    if (pair.first == source_[0]) excess += pair.second;
                    else excess -= pair.second;
                }
            }
        }
    }
    e += excess;
    return count;
}

int PInternalBlock::max_excess(int i, int j, int& e) {
    if (i == 0 && (j == length()-1 || j == source_.length() - start_index_ - 1)) {
        for (auto pair : ranks_) {
            if (pair.first == source_[0]) e += pair.second;
            else e -= pair.second;
        }
        return max_prefix_excess_;
    }

    int M = -1;
    int cumulative_length = 0;
    bool first_search_done = false;
    int excess = 0;
    for (PBlock* child : children_) {
        cumulative_length += child->length();
        if (!first_search_done) {
            if (i < cumulative_length) {
                if (j < cumulative_length) {
                    M = child->max_excess(i - (cumulative_length - child->length()),
                                          j - (cumulative_length - child->length()), excess);
                    break;
                }
                // Suffix min_excess
                M = child->max_excess(i - (cumulative_length - child->length()), child->length()-1, excess);
                first_search_done = true;
            }
        } else {
            if (j < cumulative_length) {
                int be = excess;
                // Prefix min_excess
                int max = child->max_excess(0,
                                            j - (cumulative_length - child->length()), excess);
                M = (M > max + be) ? M : max+be;
                break;
            } else {
                M = (M > excess + child->max_prefix_excess_) ? M : excess + child->max_prefix_excess_;
                for (auto pair : child->ranks_) {
                    if (pair.first == source_[0]) excess += pair.second;
                    else excess -= pair.second;
                }
            }
        }
    }
    e += excess;
    return M;
}


int PInternalBlock::add_rank_select_leaf_support() {
    starts_with_end_leaf_ = source_[start_index_] != source_[0] && source_[start_index_-1] == source_[0];

    for (PBlock* child: children_) {
        child->prefix_leaf_rank_ = leaf_rank_;
        leaf_rank_ += child->add_rank_select_leaf_support();
        child->cumulated_leaf_rank_ = leaf_rank_;
    }
    return leaf_rank_;
}
int PInternalBlock::leaf_rank(int i) {
    int cumulative_length = 0;
    for (PBlock* child: children_) {
        cumulative_length += child->length();
        if (i < cumulative_length) return child->prefix_leaf_rank_ + child->leaf_rank(i-(cumulative_length-child->length()));
    }
    return 0;
}

int PInternalBlock::leaf_rank_alternative(int i) {
    int cumulative_length = 0;
    int r = 0;
    for (PBlock* child: children_) {
        cumulative_length += child->length();
        if (i < cumulative_length) return r + child->leaf_rank_alternative(i-(cumulative_length-child->length()));
        r += child->leaf_rank_;
    }
    return 0;
}

int PInternalBlock::better_leaf_rank(int i) {
    int cumulative_length = 0;
    for (PBlock* child: children_) {
        cumulative_length += child->length();
        if (i < cumulative_length) return ((child->child_number_ == 0)? 0 : child->parent_->children_[child->child_number_-1]->cumulated_leaf_rank_) + child->better_leaf_rank(i-(cumulative_length-child->length()));
    }
    return 0;
}

int PInternalBlock::leaf_select(int j) {
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


int PInternalBlock::leaf_select_alternative(int j) {
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

int PInternalBlock::better_leaf_select(int j) {
    int cumulative_length = 0;
    int r = 0;
    for (PBlock* child: children_) {
        if (j <= child->cumulated_leaf_rank_) return cumulative_length + child->better_leaf_select(j-r);
        cumulative_length += child->length();
        r = child->cumulated_leaf_rank_;
    }
    return -1;
}


int PInternalBlock::test_fwdsearch(int i, int d, int& e, int& c) {
    ++c;
    int a = -1;
    int cumulative_length = 0;
    bool first_search_done = false;
    for (PBlock* child: children_) {
        cumulative_length += child->length();
        if (!first_search_done) {
            if (i + 1 < cumulative_length) {
                a = child->test_fwdsearch(i - (cumulative_length - child->length()), d, e,c);
                if (a != -1) a += cumulative_length - child->length();
                first_search_done = true;
            }
        } else {
            if (a == -1 && e + child->min_prefix_excess_ <= d) {
                a = child->test_fwdsearch(-1, d, e,c); // Secure seach
                a += cumulative_length - child->length();
            }
            else {
                for (auto pair : child->ranks_) {
                    if (pair.first == source_[0]) e += pair.second;
                    else e -= pair.second;
                }
            }
        }
    }
    return a;
}


int PInternalBlock::test_fwdsearch(int i, int j, int d, int& e, int& c) {
    ++c;
    int a = -1;
    int cumulative_length = 0;
    bool first_search_done = false;
    for (PBlock* child: children_) {
        cumulative_length += child->length();
        if (!first_search_done) {
            if (i + 1 < cumulative_length) {
                if (j  < cumulative_length) {
                    a = child->test_fwdsearch(i - (cumulative_length - child->length()), j - (cumulative_length - child->length()), d, e,c);
                    if (a != -1) {
                        a += cumulative_length - child->length();
                    }
                    break;
                }
                a = child->test_fwdsearch(i - (cumulative_length - child->length()), child->length()-1, d, e,c);
                if (a != -1) {
                    a += cumulative_length - child->length();
                    break;
                }
                first_search_done = true;
            }
        } else {
            if (j  < cumulative_length) {
                if (j == cumulative_length-1) {
                    if (e + child->min_prefix_excess_ <= d) {
                        a = child->test_fwdsearch(-1, j - (cumulative_length - child->length()), d, e, c);
                        if (a != -1) {
                            a += cumulative_length - child->length();
                        }
                    } else {
                        for (auto pair : child->ranks_) {
                            if (pair.first == source_[0]) e += pair.second;
                            else e -= pair.second;
                        }
                    }
                }
                else {
                    /*
                    //NUEVO
                    if (e + child->min_prefix_excess_ > d) {
                        e += 2 * child->test_rank(source_[0], j - (cumulative_length - child->length()), c) -
                             (j - (cumulative_length - child->length())) - 1;
                    }
                    //NUEVO
                    else {
                     */
                        a = child->test_fwdsearch(-1, j - (cumulative_length - child->length()), d, e, c);
                        if (a != -1) {
                            a += cumulative_length - child->length();
                        }
                    //}
                }
                break;
            } else {
                if (e + child->min_prefix_excess_ <= d) {
                    a = child->test_fwdsearch(-1, child->length()-1, d, e,c); //Secure search
                    a += cumulative_length - child->length();
                    break;
                }
                else {
                    for (auto pair : child->ranks_) {
                        if (pair.first == source_[0]) e += pair.second;
                        else e -= pair.second;
                    }
                }
            }
        }
    }
    return a;
}

int PInternalBlock::test_bwdsearch(int i, int j, int d, int& e, int& c) { //Let's supposse i<=j
    ++c;
    int a = source_.length();
    int cumulative_length = 0;
    for (PBlock* child: children_) {
        cumulative_length += child->length();
    }
    bool first_search_done = false;
    for (auto rit = children_.rbegin(); rit != children_.rend(); rit++) {
        PBlock* child = (*rit);
        cumulative_length -= child->length();
        if (!first_search_done) {
            if (j >= cumulative_length) {
                if (i >= cumulative_length) {
                    a = child->test_bwdsearch(i - (cumulative_length - child->length()), j - (cumulative_length - child->length()), d, e,c);
                    if (a != source_.length()) a += cumulative_length;
                    break;
                }
                a = child->test_bwdsearch(0, j - cumulative_length, d, e,c);
                if (a != source_.length()) {
                    a += cumulative_length;
                    break;
                }
                first_search_done = true;
            }
        } else {
            if (i >= cumulative_length) {
                // TRANSLATE
                if (i == cumulative_length) {
                    int child_excess = 0;
                    for (auto pair : child->ranks_) {
                        if (pair.first == source_[0]) child_excess += pair.second;
                        else child_excess -= pair.second;
                    }
                    int reached = ((child_excess-child->min_prefix_excess_) > child_excess) ? (child_excess-child->min_prefix_excess_) : child_excess;

                    if (e + reached >= -d) {
                        a = child->test_bwdsearch(i - cumulative_length,child->length()-1, d, e,c); // Secure seach
                        a += cumulative_length;
                        break;
                    } else {
                        for (auto pair : child->ranks_) {
                            if (pair.first == source_[0]) e += pair.second;
                            else e -= pair.second;
                        }
                    }
                }
                else {
                    /*
                    //NUEVO
                    if (e + child->min_prefix_excess_ > d) {
                        e += 2 * child->test_rank(source_[0], j - (cumulative_length - child->length()), c) -
                             (j - (cumulative_length - child->length())) - 1;
                    }
                    //NUEVO
                    else {
                     */
                    a = child->test_bwdsearch(i - cumulative_length,child->length()-1, d, e,c); // Secure seach
                    if (a != -1) {
                        a += cumulative_length - child->length();
                    }
                    //}
                }
                break;
            } else {
                int child_excess = 0;
                for (auto pair : child->ranks_) {
                    if (pair.first == source_[0]) child_excess += pair.second;
                    else child_excess -= pair.second;
                }
                int reached = ((child_excess-child->min_prefix_excess_) > child_excess) ? (child_excess-child->min_prefix_excess_) : child_excess;

                if (e + reached >= -d) {
                    a = child->test_bwdsearch(0,child->length()-1, d, e,c); // Secure seach
                    a += cumulative_length;
                    break;
                }
                else {
                    for (auto pair : child->ranks_) {
                        if (pair.first == source_[0]) e += pair.second;
                        else e -= pair.second;
                    }
                }
            }
        }
    }
    return a;
}


int PInternalBlock::test_bwdsearch(int i, int d, int& e, int& c) {
    ++c;
    int a = source_.length();
    int cumulative_length = 0;
    for (PBlock* child: children_) {
        cumulative_length += child->length();
    }
    bool first_search_done = false;
    for (auto rit = children_.rbegin(); rit != children_.rend(); rit++) {
        PBlock* child = (*rit);
        cumulative_length -= child->length();
        if (!first_search_done) {
            if (i >= cumulative_length) {
                a = child->test_bwdsearch(i - cumulative_length, d, e,c);
                if (a != source_.length()) a += cumulative_length;
                first_search_done = true;
            }
        } else {
            int child_excess = 0;
            for (auto pair : child->ranks_) {
                if (pair.first == source_[0]) child_excess += pair.second;
                else child_excess -= pair.second;
            }
            int reached = ((child_excess-child->min_prefix_excess_) > child_excess) ? (child_excess-child->min_prefix_excess_) : child_excess;
            if (a == source_.length() && e + reached >= -d) {
                a = child->test_bwdsearch(child->length()-1, d, e,c); // Secure seach
                a += cumulative_length;
            }
            else {
                for (auto pair : child->ranks_) {
                    if (pair.first == source_[0]) e += pair.second;
                    else e -= pair.second;
                }
            }
        }
    }
    return a;
}


int PInternalBlock::test_min_excess(int i, int j, int& e, int& c) {

    if (i == 0 && (j == length()-1 || j == source_.length() - start_index_ - 1)) {
        for (auto pair : ranks_) {
            if (pair.first == source_[0]) e += pair.second;
            else e -= pair.second;
        }
        return min_prefix_excess_;
    }

    ++c;

    int m = 2;
    int cumulative_length = 0;
    bool first_search_done = false;
    int excess = 0;
    for (PBlock* child : children_) {
        cumulative_length += child->length();
        if (!first_search_done) {
            if (i < cumulative_length) {
                if (j < cumulative_length) {
                    m = child->test_min_excess(i - (cumulative_length - child->length()),
                                          j - (cumulative_length - child->length()), excess,c);
                    break;
                }
                // Suffix min_excess
                m = child->test_min_excess(i - (cumulative_length - child->length()), child->length()-1, excess,c);
                first_search_done = true;
            }
        } else {
            if (j < cumulative_length) {
                int be = excess;
                // Prefix min_excess
                int min = child->test_min_excess(0,
                                            j - (cumulative_length - child->length()), excess,c);
                m = (m < min + be) ? m : min+be;
                break;
            } else {
                m = (m < excess + child->min_prefix_excess_) ? m : excess + child->min_prefix_excess_;
                for (auto pair : child->ranks_) {
                    if (pair.first == source_[0]) excess += pair.second;
                    else excess -= pair.second;
                }
            }
        }
    }
    e += excess;
    return m;
}

void PInternalBlock::put_fix_references() {
    for (PBlock* b: children_) {
        b->put_fix_references();
    }
}