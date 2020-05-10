//
// Created by sking32 on 5/18/18.
//

#include "padding_blocktree/PBlockTree.h"

#include <queue>
#include <iostream>
#include <padding_blocktree/PLazyInternalBlock.h>
#include <padding_blocktree/PLeafBlock.h>
#include <stack>

#include "padding_blocktree/PInternalBlock.h"


PBlockTree::PBlockTree(std::string& input, int r, int leaf_length): r_(r), input_(input), leaf_length_(leaf_length) {

    if (input_.size() <= leaf_length_ || input_.size()<r)
        root_block_ = new PLeafBlock(nullptr, 0, input_.size()-1, r, leaf_length, input_, 0);
    else {
        int number_of_leaves = (input_.size()%leaf_length_ == 0) ? input_.size()/leaf_length_ : input_.size()/leaf_length_+1;
        int height =  0;

        int nl = number_of_leaves-1;
        int64_t block_length = leaf_length_;
        while (nl){
            height++;
            block_length*=r_;
            nl/=r_;
        }

        root_block_ = new PLazyInternalBlock(nullptr, 0, block_length-1, r, leaf_length, input_, 0);
    }

}

PBlockTree::~PBlockTree() {
    delete root_block_;
}


void PBlockTree::add_rank_select_support(int c) {
    root_block_->add_rank_select_support(c);
}

int PBlockTree::rank(int c, int i) {
    return root_block_->rank(c, i);
}

int PBlockTree::rank_alternative(int c, int i) {
    return root_block_->rank_alternative(c, i);
}

int PBlockTree::better_rank(int c, int i) {
    return root_block_->better_rank(c, i);
}

int PBlockTree::select(int c, int j) {
    return root_block_->select(c, j);
}

int PBlockTree::select_alternative(int c, int j) {
    return root_block_->select_alternative(c, j);
}

int PBlockTree::better_select(int c, int j) {
    return root_block_->better_select(c, j);
}

void PBlockTree::print_statistics() {
    std::cout << "Number of nodes : " << number_of_nodes() << std::endl;
    std::cout << "Number of leaves : " << number_of_leaves() << std::endl;
    std::cout << "Number of InternalBlocks : " << number_of_internal_blocks() << std::endl;
    std::cout << "Number of BackBlocks : " << number_of_back_blocks() << std::endl;
    std::cout << "Number of LeafBlocks : " << number_of_leaf_blocks() << std::endl;
    std::cout << std::endl;

    bool ok = true;
    for (int i = 0; i < input_.size(); ++i) {
        ok = ok && (access(i) == (int) input_[i]);
    }
    if (!ok) std::cout << "Error" << std::endl;

}

std::vector<std::vector<PBlock*>> PBlockTree::levelwise_iterator() {
    std::vector<std::vector<PBlock*>> result = {{root_block_}};
    while (!dynamic_cast<PLeafBlock*>(result.back()[0])) {
        std::vector<PBlock*> next_level = {};
        for (PBlock *b : result.back())
            for (PBlock *child : b->children())
                next_level.push_back(child);
        result.push_back(next_level);
    }

    return result;
}

int PBlockTree::number_of_nodes() {
    return root_block_->number_of_nodes();
}

int PBlockTree::number_of_back_blocks() {
    return root_block_->number_of_back_blocks();
}

int PBlockTree::number_of_internal_blocks() {
    return root_block_->number_of_internal_blocks();
}

int PBlockTree::number_of_leaf_blocks() {
    return root_block_->number_of_leaf_blocks();
}

int PBlockTree::height() {
    return root_block_->height();
}

int PBlockTree::number_of_leaves() {
    return root_block_->number_of_leaves();
}

void PBlockTree::clean_unnecessary_expansions() {
    root_block_->clean_unnecessary_expansions();
    for (std::vector<PBlock*> level : levelwise_iterator()) {
        for (int i = 0; i<level.size(); ++i) {
            level[i]->level_index_ = i;
            level[i]->first_occurrence_level_index_ = level[i]->first_block_->level_index_;
        }
    }
}

void PBlockTree::clean_unnecessary_expansions(int c) {
    root_block_->put_fix_references();
    root_block_->clean_unnecessary_expansions(c);
    for (std::vector<PBlock*> level : levelwise_iterator()) {
        std::priority_queue<PBlock*, std::vector<PBlock*>, pblockcomparison> q;
        for (int i = 0 ; i < level.size(); ++i) {
            PBlock* b = level[i];
            if (b->fix_reference_ != -1)
                q.push(b);
            b->level_index_ = i;
            b->first_occurrence_level_index_ = i;
        }

        int i = 0;
        while (!q.empty()) {
            PBlock * bb = q.top();
            q.pop();
            for (; level[i]->start_index_ < bb->fix_reference_; ++i);
            bb->first_occurrence_level_index_ = i;
            bb->first_block_ = level[i];
            if (bb->second_block_ != nullptr) bb->second_block_ = level[i+1];
        }

    }
}


int PBlockTree::access(int i) {
    return root_block_->access(i);
}

int PBlockTree::access_2(int i, int& a) {
    return root_block_->access_2(i,a);
}

int PBlockTree::number_of_selfreferences() {
    return root_block_->number_of_selfreferences();
}

bool PBlockTree::check_heuristic() {
    return root_block_->check_heuristic();
}

bool PBlockTree::check() {
    return root_block_->check();
}

bool PBlockTree::contains_double_pointer() {
    return root_block_->contains_double_pointer(false);
}

int PBlockTree::get_max_hop() {
    return root_block_->compute_max_hop();
}


std::vector<PBlock*> PBlockTree::next_level(std::vector<PBlock*>& level) {
    std::vector<PBlock*> next_level;
    for (int i = 0; i < level.size(); ++i) {
        PBlock* b = level[i];
        for (PBlock *child : b->children()) { // Do it in order
            child->level_index_ = next_level.size();
            child->first_occurrence_level_index_ = next_level.size();
            next_level.push_back(child);
        }
    }
    return next_level;
}

void backward_window_block_scan(std::vector<PBlock*>& level, int window_size, PBlockTree* bt, int N, std::unordered_map<HashString, std::vector<PBlock*>>& hashtable) {
    //std::cout << "RUNNING WINDOW_BLOCK SCANNING for length: " << window_size << std::endl;
    for (std::vector<PBlock *>::reverse_iterator rit = level.rbegin(); rit != level.rend();) {
        PBlock *b = (*rit);
        BackwardRabinKarp* brk;
        int offset = 0;
        int cumulative_length = 0;

        for (;(rit != level.rend() && ((*rit) == b || (*(rit-1))->start_index_ == (*rit)->end_index_ + 1)) ; rit++) {

            if (cumulative_length+(*rit)->length() >= window_size) {
                offset = (*rit)->length() - (window_size - cumulative_length);
                break;
            }
            cumulative_length += (*rit)->length();
        }
        if (rit != level.rend())  brk = new BackwardRabinKarp(bt->input_, (*rit)->start_index_ + offset, window_size ,N);


        for (;rit != level.rend() && ((*rit) == b || (*(rit-1))->start_index_ == (*rit)->end_index_ + 1); rit++) {
            for (; offset>=0; --offset) {
                PBlock* current = (*rit);
                HashString hS(brk->hash(), bt->input_, current->start_index_ + offset, current->start_index_ + offset + window_size - 1);
                std::unordered_map<HashString, std::vector<PBlock *>>::const_iterator result = hashtable.find(hS);
                if (result != hashtable.end()) {
                    std::vector<PBlock*> blocks = result->second;
                    for (PBlock* b : blocks) {
                        if (current->start_index_ + offset <= b->start_index_) {
                            //b->print();
                            b->first_block_ = current;
                            b->offset_ = offset;
                            if (offset + window_size > b->first_block_->length()) b->second_block_ = (*(rit-1));
                            else b->second_block_ = nullptr;
                        }
                    }
                }

                if (current->start_index_ + offset > 0) brk->next();
            }
            if (rit+1 != level.rend()) offset = (*(rit+1))->length()-1;
        }
    }
}


void backward_pair_window_block_scan(std::vector<PBlock*>& level, int pair_window_size, PBlockTree* bt, int N, std::unordered_map<HashString, std::vector<std::pair<PBlock*, PBlock*>>>& pair_hashtable) {
    for (std::vector<PBlock *>::reverse_iterator rit = level.rbegin(); rit != level.rend();) {
        PBlock *b = (*rit);
        b->left_ = true;
        BackwardRabinKarp* brk;
        int offset = 0;
        int cumulative_length = 0;

        for (;(rit != level.rend() && ((*rit) == b || (*(rit-1))->start_index_ == (*rit)->end_index_ + 1)) ; rit++) {

            if (cumulative_length+(*rit)->length() >= pair_window_size) {
                offset = (*rit)->length() - (pair_window_size - cumulative_length);
                break;
            }
            cumulative_length += (*rit)->length();
        }
        if (rit != level.rend()) brk = new BackwardRabinKarp(bt->input_, (*rit)->start_index_ + offset, pair_window_size ,N);

        for (;rit != level.rend() && ((*rit) == b || (*(rit-1))->start_index_ == (*rit)->end_index_ + 1); rit++) {
            for (; offset>=0; --offset) {
                PBlock* current = (*rit);
                HashString hS(brk->hash(), bt->input_, current->start_index_ + offset, current->start_index_ + offset + pair_window_size - 1);
                std::unordered_map<HashString, std::vector<std::pair<PBlock *,PBlock*>>>::const_iterator result = pair_hashtable.find(hS);
                if (result != pair_hashtable.end()) {
                    for (std::pair<PBlock*,PBlock*> p: result->second) {
                        if (current->start_index_ + offset < p.first->start_index_) {
                            p.first->left_ = true;
                            p.second->right_ = true;
                        }
                    }
                }

                if (current->start_index_ + offset > 0) brk->next();
            }
            if (rit+1 != level.rend()) offset = (*(rit+1))->length()-1;
        }
        (*(rit-1))->right_ = true;
    }
}

void PBlockTree::forward_pair_window_block_scan(std::vector<PBlock*>& level, int pair_window_size, int N, std::unordered_map<HashString, std::vector<std::pair<PBlock*, PBlock*>>>& pair_hashtable) {
    for (std::vector<PBlock *>::iterator it = level.begin(); it != level.end();) {
        PBlock *b = (*it);
        b->right_ = true;
        int offset = 0;
        RabinKarp rk(input_, (*it)->start_index_ + offset, pair_window_size, N); // offset is always 0 here
        for (; it != level.end() && ((*it) == b || (*(it-1))->end_index_ == (*it)->start_index_ - 1); it++) {
            PBlock* current = *(it);
            bool last_block = ((it+1) == level.end() ||  current->end_index_ != (*(it+1))->start_index_ - 1);
            for (offset = 0; offset < current->length(); ++offset) {
                if (last_block && current->length() - offset < pair_window_size)  break;
                HashString hS(rk.hash(), input_, current->start_index_ + offset, current->start_index_ + offset + pair_window_size - 1);
                std::unordered_map<HashString, std::vector<std::pair<PBlock*, PBlock*>>>::const_iterator result = pair_hashtable.find(hS);
                if (result != pair_hashtable.end()) { // Here, It could be that the scanning should have finished with the penultimate, but it never should enter this ''if''
                                                        // when We're on the penultimate block and the window exceeds the last block because if that is a first occurrence should have been occured before in a pair of blocks
                                                        // maybe use a condition more like rk's condition below could work fine too
                                                        // Same logic: for when passing a window of size 2l + 2 over 2 block of length l
                    for (std::pair<PBlock*,PBlock*> p: result->second) {
                        if (current->start_index_ + offset < p.first->start_index_) {
                            p.first->left_ = true;
                            p.second->right_ = true;
                        }
                    }
                    pair_hashtable.erase(hS);
                }
                if (current->start_index_+offset+pair_window_size < input_.size()) rk.next();
            }
        }
        (*(it-1))->left_ = true;
    }
}


void PBlockTree::forward_window_block_scan(std::vector<PBlock*>& level, int window_size, int N, std::unordered_map<HashString, std::vector<PBlock*>>& hashtable) {
    int i = 0;
    for (std::vector<PBlock *>::iterator it = level.begin(); it != level.end();) {
        PBlock *b = (*it);
        int offset = 0;
        RabinKarp rk(input_, (*it)->start_index_ + offset, window_size, N);
        for (; it != level.end() && ((*it) == b || (*(it-1))->end_index_ == (*it)->start_index_ - 1); it++, i++) {
            PBlock* current = *(it);
            bool last_block = ((it+1) == level.end() ||  current->end_index_ != (*(it+1))->start_index_ - 1);
            for (offset = 0; offset < current->length(); ++offset) {
                if (last_block && current->length() - offset < window_size)  break;
                HashString hS(rk.hash(), input_, current->start_index_ + offset, current->start_index_ + offset + window_size - 1);
                std::unordered_map<HashString, std::vector<PBlock *>>::const_iterator result = hashtable.find(hS);
                if (result != hashtable.end()) {
                    std::vector<PBlock*> blocks = result->second;
                    for (PBlock* b : blocks) {
                        b->first_occurrence_level_index_ = i;
                        b->first_block_ = current;
                        b->offset_ = offset;
                        if (offset + window_size > b->first_block_->length()) b->second_block_ = (*(it+1));
                        else b->second_block_ = nullptr;
                    }
                    hashtable.erase(hS);
                }
                if (current->start_index_+offset+window_size < input_.size()) rk.next();
            }
        }
    }
}


void PBlockTree::forward_window_block_scan_real_conservative_heuristic(std::vector<PBlock*>& level, int window_size, int N, std::unordered_map<HashString, std::vector<PBlock*>>& hashtable, int c) {
    int i = 0;
    for (std::vector<PBlock *>::iterator it = level.begin(); it != level.end();) {
        PBlock *b = (*it);
        int offset = 0;
        RabinKarp rk(input_, (*it)->start_index_ + offset, window_size, N);
        for (; it != level.end() && ((*it) == b || (*(it-1))->end_index_ == (*it)->start_index_ - 1); it++, i++) {
            PBlock* current = *(it);
            bool last_block = ((it+1) == level.end() ||  current->end_index_ != (*(it+1))->start_index_ - 1);
            for (offset = 0; offset < current->length(); ++offset) {
                if (last_block && current->length() - offset < window_size)  break;
                HashString hS(rk.hash(), input_, current->start_index_ + offset, current->start_index_ + offset + window_size - 1);
                std::unordered_map<HashString, std::vector<PBlock *>>::const_iterator result = hashtable.find(hS);
                if (result != hashtable.end()) {
                    std::vector<PBlock*> blocks = result->second;
                    std::vector<PBlock*> new_blocks = {};
                    for (PBlock* b : blocks) {
                        if (offset + window_size > b->first_block_->length() && b == (*(it+1))) new_blocks.push_back(b);
                        else {
                            int color = current->color_+1;
                            if (offset + window_size > b->first_block_->length()) {
                                if (color < (*(it + 1))->color_+1) color = (*(it + 1))->color_+1;
                            }
                            if (current == b) color = 0;
                            if (color > c) {
                                new_blocks.push_back(b);
                            } else {
                                b->color_ = color;
                                b->first_occurrence_level_index_ = i;
                                b->first_block_ = current;
                                b->offset_ = offset;
                                if (offset + window_size > b->first_block_->length()) b->second_block_ = (*(it + 1));
                                else b->second_block_ = nullptr;
                            }
                        }
                    }
                    if (new_blocks.size() == 0) hashtable.erase(hS);
                    else hashtable[hS] = new_blocks;
                }
                if (current->start_index_+offset+window_size < input_.size()) rk.next();
            }
        }
    }
}


void PBlockTree::forward_window_block_scan_real_conservative_optimized_heuristic(std::vector<PBlock*>& level,
                                                                                 int window_size, int N, std::unordered_map<HashString, std::vector<PBlock*>>& hashtable) {
    int i = 0;
    for (std::vector<PBlock *>::iterator it = level.begin(); it != level.end();) {
        PBlock *b = (*it);
        int offset = 0;
        RabinKarp rk(input_, (*it)->start_index_ + offset, window_size, N);
        for (; it != level.end() && ((*it) == b || (*(it-1))->end_index_ == (*it)->start_index_ - 1); it++, i++) {
            PBlock* current = *(it);
            bool last_block = ((it+1) == level.end() ||  current->end_index_ != (*(it+1))->start_index_ - 1);
            for (offset = 0; offset < current->length(); ++offset) {
                if (last_block && current->length() - offset < window_size)  break;
                HashString hS(rk.hash(), input_, current->start_index_ + offset, current->start_index_ + offset + window_size - 1);
                std::unordered_map<HashString, std::vector<PBlock *>>::const_iterator result = hashtable.find(hS);
                if (result != hashtable.end()) {
                    std::vector<PBlock*> blocks = result->second;
                    std::vector<PBlock*> new_blocks = {};
                    for (PBlock* b : blocks) {
                        if (offset + window_size > b->first_block_->length() && b == (*(it+1))) new_blocks.push_back(b);
                        else {
                            if (b != current) {
                                if (offset + window_size <= b->first_block_->length()) {
                                    if (current->first_block_ != current) {
                                        current->first_block_->pointing_to_me_--;
                                        if (current->second_block_ != nullptr)
                                            current->second_block_->pointing_to_me_--;

                                        current->first_occurrence_level_index_ = current->level_index_;
                                        current->first_block_ = current;
                                        current->second_block_ = nullptr;
                                    }
                                    current->pointing_to_me_++;
                                    b->first_occurrence_level_index_ = i;
                                    b->first_block_ = current;
                                    b->offset_ = offset;
                                    b->second_block_ = nullptr;
                                } else if (current->first_block_ == current || (*(it + 1))->first_block_ == (*(it + 1))){
                                    if (current->first_block_ != current) {
                                        current->first_block_->pointing_to_me_--;
                                        if (current->second_block_ != nullptr)
                                            current->second_block_->pointing_to_me_--;
                                        current->first_occurrence_level_index_ = current->level_index_;
                                        current->first_block_ = current;
                                        current->second_block_ = nullptr;
                                    }
                                    if ((*(it + 1))->first_block_ != (*(it + 1))) {
                                        (*(it + 1))->first_block_->pointing_to_me_--;
                                        if ((*(it + 1))->second_block_ != nullptr)
                                            (*(it + 1))->second_block_->pointing_to_me_--;
                                        (*(it + 1))->first_occurrence_level_index_ = (*(it + 1))->level_index_;
                                        (*(it + 1))->first_block_ = (*(it + 1));
                                        (*(it + 1))->second_block_ = nullptr;
                                    }
                                    current->pointing_to_me_++;
                                    (*(it + 1))->pointing_to_me_++;
                                    b->first_occurrence_level_index_ = i;
                                    b->first_block_ = current;
                                    b->offset_ = offset;
                                    b->second_block_ = (*(it + 1));
                                } else {
                                    if (current->pointing_to_me_ + (*(it+1))->pointing_to_me_ > 0) {
                                        current->first_block_->pointing_to_me_--;
                                        if (current->second_block_ != nullptr)
                                            current->second_block_->pointing_to_me_--;

                                        (*(it + 1))->first_block_->pointing_to_me_--;
                                        if ((*(it + 1))->second_block_ != nullptr)
                                            (*(it + 1))->second_block_->pointing_to_me_--;
                                        current->pointing_to_me_++;
                                        (*(it + 1))->pointing_to_me_++;
                                        b->first_occurrence_level_index_ = i;
                                        b->first_block_ = current;
                                        b->offset_ = offset;
                                        b->second_block_ = (*(it + 1));
                                    } else {
                                        new_blocks.push_back(b);
                                    }
                                }
                            }
                        }
                    }
                    if (new_blocks.size() == 0) hashtable.erase(hS);
                    else hashtable[hS] = new_blocks;
                }
                if (current->start_index_+offset+window_size < input_.size()) rk.next();
            }
        }
    }
}

void PBlockTree::forward_window_block_scan_front(std::vector<PBlock*>& level, int window_size, int N, std::unordered_map<HashString, std::vector<PBlock*>>& hashtable) {
    int i = 0;
    for (std::vector<PBlock *>::iterator it = level.begin(); it != level.end();) {
        PBlock *b = (*it);
        int offset = 0;
        RabinKarp rk(input_, (*it)->start_index_ + offset, window_size, N);
        for (; it != level.end() && ((*it) == b || (*(it-1))->end_index_ == (*it)->start_index_ - 1); it++, i++) {
            PBlock* current = *(it);
            bool last_block = ((it+1) == level.end() ||  current->end_index_ != (*(it+1))->start_index_ - 1);
            for (offset = 0; offset < current->length(); ++offset) {
                if (last_block && current->length() - offset < window_size)  break;
                HashString hS(rk.hash(), input_, current->start_index_ + offset, current->start_index_ + offset + window_size - 1);
                std::unordered_map<HashString, std::vector<PBlock *>>::const_iterator result = hashtable.find(hS);
                if (result != hashtable.end()) {
                    std::vector<PBlock*> blocks = result->second;
                    std::vector<PBlock*> new_blocks = {};
                    for (PBlock* b : blocks) {
                        if (b == current && offset == 0) new_blocks.push_back(b);
                        b->first_occurrence_level_index_ = i;
                        b->first_block_ = current;
                        b->offset_ = offset;
                        if (offset + window_size > b->first_block_->length()) b->second_block_ = (*(it+1));
                        else b->second_block_ = nullptr;
                    }
                    if (new_blocks.size() == 0)hashtable.erase(hS);
                    else hashtable[hS] = new_blocks;
                }
                if (current->start_index_+offset+window_size < input_.size()) rk.next();
            }
        }
    }
}

void PBlockTree::block_scan(std::vector<PBlock *>& level, int N , std::unordered_map<HashString, std::vector<PBlock*>>& hashtable) {
    for (PBlock* b : level) {
        RabinKarp rk(input_, b->start_index_, b->length(), N);
        HashString hS(rk.hash(),  input_, b->start_index_, b->end_index_);

        std::unordered_map<HashString, std::vector<PBlock*>>::const_iterator result = hashtable.find(hS);

        if (result == hashtable.end())
            hashtable[hS] = {b};
        else
            hashtable[hS].push_back(b);
    }
}


void PBlockTree::process_level(std::vector<PBlock*>& level) {

    int N = 6700417; //Large prime
    int level_length = level.front()->length();

    // Block scan
    std::unordered_map<HashString, std::vector<PBlock*>> hashtable;
    block_scan(level, N, hashtable);

    // Pairs of blocks scan
    std::unordered_map<HashString, std::vector<std::pair<PBlock *,PBlock*>>> pair_hashtable;
    for (std::vector<PBlock *>::iterator it = level.begin(); it != level.end();) {
        for (++it; (it != level.end() && (*(it-1))->end_index_ == (*it)->start_index_ - 1); ++it) {
            PBlock* current = (*(it-1));
            PBlock* next = (*it);
            RabinKarp rk(input_, current->start_index_, current->length() + next->length(), N);
            HashString hS(rk.hash(), input_, current->start_index_, current->start_index_ + current->length() + next->length()-1); // Second parameter is next->end_index

            std::unordered_map<HashString, std::vector<std::pair<PBlock *,PBlock*>>>::const_iterator result = pair_hashtable.find(hS);

            if (result == pair_hashtable.end())
                pair_hashtable[hS] = {{current, next}};
            else
                pair_hashtable[hS].push_back({current, next});
        }
    }


    // Window block scan
    //Establishes first occurrences of blocks
    forward_window_block_scan(level, level_length, N, hashtable);



    // Window Pair of blocks scans
    if (level.size() > 1)
        forward_pair_window_block_scan(level, level_length*2, N, pair_hashtable);




    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        PBlock* b = level[i];
        if (b->left_ && b->right_ && b->first_occurrence_level_index_ < b->level_index_) {
            // This doesn't have the bug of the dangling reference fixed with first_occurrence_level_index, because it shouldn't happen that
            // A block points back to a BackBlock
            PBackBlock* bb = new PBackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->leaf_length_, input_,
                                          b->child_number_, level[b->first_occurrence_level_index_], (b->second_block_ ==
                                                                                                      nullptr) ? nullptr : level[b->first_occurrence_level_index_ +1], b->offset_);
            bb->level_index_ = b->level_index_;
            bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
            bb->left_ = true;
            bb->right_ = true;
            //COLOR
            bb->first_block_->color_ = 1;
            if (bb->second_block_ != nullptr) bb->second_block_->color_ = 1;
            //COLOR
            b->parent_->put_child(b->child_number_, bb);
            level[i] = bb;
        }
    }

}


void PBlockTree::forward_window_block_scan_real_other_back_and_front(std::vector<PBlock*>& level, int window_size, int N, std::unordered_map<HashString, std::vector<PBlock*>>& hashtable) {
    int i = 0;
    for (std::vector<PBlock *>::iterator it = level.begin(); it != level.end();) {
        PBlock *b = (*it);
        int offset = 0;
        RabinKarp rk(input_, (*it)->start_index_ + offset, window_size, N);
        for (; it != level.end() && ((*it) == b || (*(it-1))->end_index_ == (*it)->start_index_ - 1); it++, i++) {
            PBlock* current = *(it);
            bool last_block = ((it+1) == level.end() ||  current->end_index_ != (*(it+1))->start_index_ - 1);
            for (offset = 0; offset < current->length(); ++offset) {
                if (last_block && current->length() - offset < window_size)  break;
                HashString hS(rk.hash(), input_, current->start_index_ + offset, current->start_index_ + offset + window_size - 1);
                std::unordered_map<HashString, std::vector<PBlock *>>::const_iterator result = hashtable.find(hS);
                if (result != hashtable.end()) {
                    std::vector<PBlock*> blocks = result->second;
                    std::vector<PBlock*> new_blocks = {};
                    for (PBlock* b : blocks) {
                        //if (current->first_block_ != b && current->second_block_ != b && (!(offset + window_size > b->first_block_->length()) || ((*(it+1))->first_block_ != b && (*(it+1))->second_block_ != b))) {
                        if (b != current && (!(offset + window_size > b->first_block_->length()) || (*(it+1)) != b)) { //No se traslapa
                            if (!b->left_) {
                                if (/*current->start_index_ < b->start_index_ &&*/ !current->right_ &&
                                                                                   (!(offset + window_size > b->first_block_->length()) || !(*(it + 1))->right_)) {
                                    b->right_ = true;
                                    current->left_ = true;
                                    b->first_occurrence_level_index_ = i;
                                    b->first_block_ = current;
                                    b->offset_ = offset;
                                    if (offset + window_size > b->first_block_->length()) {
                                        (*(it + 1))->left_ = true;
                                        b->second_block_ = (*(it + 1));
                                    }
                                    else {
                                        b->second_block_ = nullptr;
                                    }
                                } else {
                                    new_blocks.push_back(b);
                                }
                            } else {
                                new_blocks.push_back(b);
                            }
                        } else {
                            new_blocks.push_back(b);
                        }
                        //}
                    }
                    if (new_blocks.size() == 0) hashtable.erase(hS);
                    else hashtable[hS] = new_blocks;
                }
                if (current->start_index_+offset+window_size < input_.size()) rk.next();
            }
        }
    }
}

void PBlockTree::forward_window_block_scan_real_other_conservative_heuristic(std::vector<PBlock*>& level, int window_size, int N, std::unordered_map<HashString, std::vector<PBlock*>>& hashtable) {
    int i = 0;
    for (std::vector<PBlock *>::iterator it = level.begin(); it != level.end();) {
        PBlock *b = (*it);
        int offset = 0;
        RabinKarp rk(input_, (*it)->start_index_ + offset, window_size, N);
        for (; it != level.end() && ((*it) == b || (*(it-1))->end_index_ == (*it)->start_index_ - 1); it++, i++) {
            PBlock* current = *(it);
            bool last_block = ((it+1) == level.end() ||  current->end_index_ != (*(it+1))->start_index_ - 1);
            for (offset = 0; offset < current->length(); ++offset) {
                if (last_block && current->length() - offset < window_size)  break;
                HashString hS(rk.hash(), input_, current->start_index_ + offset, current->start_index_ + offset + window_size - 1);
                std::unordered_map<HashString, std::vector<PBlock *>>::const_iterator result = hashtable.find(hS);
                if (result != hashtable.end()) {
                    std::vector<PBlock*> blocks = result->second;
                    std::vector<PBlock*> new_blocks = {};
                    for (PBlock* b : blocks) {
                        //if (current->first_block_ != b && current->second_block_ != b && (!(offset + window_size > b->first_block_->length()) || ((*(it+1))->first_block_ != b && (*(it+1))->second_block_ != b))) {
                        if (b != current && (!(offset + window_size > b->first_block_->length()) || (*(it+1)) != b)) { //No se traslapa
                            if (!b->left_) {
                                if (current->start_index_ < b->start_index_ && !current->right_ &&
                                                                                   (!(offset + window_size > b->first_block_->length()) || !(*(it + 1))->right_)) {
                                    b->right_ = true;
                                    current->left_ = true;
                                    b->first_occurrence_level_index_ = i;
                                    b->first_block_ = current;
                                    b->offset_ = offset;
                                    if (offset + window_size > b->first_block_->length()) {
                                        (*(it + 1))->left_ = true;
                                        b->second_block_ = (*(it + 1));
                                    }
                                    else {
                                        b->second_block_ = nullptr;
                                    }
                                } else {
                                    new_blocks.push_back(b);
                                }
                            } else {
                                new_blocks.push_back(b);
                            }
                        } else {
                            new_blocks.push_back(b);
                        }
                        //}
                    }
                    if (new_blocks.size() == 0) hashtable.erase(hS);
                    else hashtable[hS] = new_blocks;
                }
                if (current->start_index_+offset+window_size < input_.size()) rk.next();
            }
        }
    }
}

void PBlockTree::process_level_real_conservative_heuristic(std::vector<PBlock*>& level) {

    int N = 6700417; // Large prime
    int level_length = level.front()->length();

    // Block scan
    std::unordered_map<HashString, std::vector<PBlock*>> hashtable;
    block_scan(level, N, hashtable);

    // Window block scan
    // This is almost the same as forward_window_block_scan, as well as the BackBlock creation
    forward_window_block_scan_real_conservative_heuristic(level, level_length, N, hashtable);

    /*
    clean_self_references(level);
    update_pointing_to_me(level);
    for (auto it = level.begin(); it != level.end(); it++) {
        PBlock *current = *(it);
        if (current->first_occurrence_level_index_ > current->level_index_) {
            current->first_block_ = current;
            current->second_block_ = nullptr;
            current->first_occurrence_level_index_ = current->level_index_;
        }
        if (current->first_block_ != current) {
            current->first_block_->pointing_to_me_++;
        }
        if (current->second_block_ != nullptr && current->second_block_ != current) {
            current->second_block_->pointing_to_me_++;
        }
    }
     */

    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        PBlock* b = level[i];
        if (!b->left_ && b->first_block_ != b) {
            PBackBlock* bb = new PBackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->leaf_length_, input_,
                                            b->child_number_, level[b->first_occurrence_level_index_], (b->second_block_ ==
                                                                                                        nullptr) ? nullptr : level[b->first_occurrence_level_index_ +1], b->offset_);
            bb->first_block_->pointing_to_me_--;
            if (bb->second_block_ != nullptr) bb->second_block_->pointing_to_me_--;
            bb->level_index_ = b->level_index_;
            bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
            bb->left_ = b->left_;
            b->parent_->put_child(b->child_number_, bb); // Todo: parent is always InternalBlock
            level[i] = bb;
        }
    }
}

void PBlockTree::process_level_real_back_front(std::vector<PBlock*>& level) {

    int N = 6700417; // Large prime
    int level_length = level.front()->length();

    // Block scan
    std::unordered_map<HashString, std::vector<PBlock*>> hashtable;
    block_scan(level, N, hashtable);

    // Window block scan
    // This is almost the same as forward_window_block_scan, as well as the BackBlock creation
    forward_window_block_scan_real_back_and_front(level, level_length, N, hashtable);

    /*
    clean_self_references(level);
    update_pointing_to_me(level);
    for (auto it = level.begin(); it != level.end(); it++) {
        PBlock *current = *(it);
        if (current->first_occurrence_level_index_ > current->level_index_) {
            current->first_block_ = current;
            current->second_block_ = nullptr;
            current->first_occurrence_level_index_ = current->level_index_;
        }
        if (current->first_block_ != current) {
            current->first_block_->pointing_to_me_++;
        }
        if (current->second_block_ != nullptr && current->second_block_ != current) {
            current->second_block_->pointing_to_me_++;
        }
    }
     */

    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        PBlock* b = level[i];
        if (!b->left_ && b->first_block_ != b) {
            PBackBlock* bb = new PBackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->leaf_length_, input_,
                                            b->child_number_, level[b->first_occurrence_level_index_], (b->second_block_ ==
                                                                                                        nullptr) ? nullptr : level[b->first_occurrence_level_index_ +1], b->offset_);
            bb->first_block_->pointing_to_me_--;
            if (bb->second_block_ != nullptr) bb->second_block_->pointing_to_me_--;
            bb->level_index_ = b->level_index_;
            bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
            bb->left_ = b->left_;
            b->parent_->put_child(b->child_number_, bb); // Todo: parent is always InternalBlock
            level[i] = bb;
        }
    }

}

void PBlockTree::process_level_back_front(std::vector<PBlock*>& level) {

    int N = 6700417; // Large prime
    int level_length = level.front()->length();

    // Block scan
    std::unordered_map<HashString, std::vector<PBlock*>> hashtable;
    block_scan(level, N, hashtable);

    // Window block scan
    // This is almost the same as forward_window_block_scan, as well as the BackBlock creation
    forward_window_block_scan_front(level, level_length, N, hashtable);
    for (int i = 0; i < level.size(); ++i) {
        PBlock *b = level[i];
        if (b == b->first_block_) b->left_ = true;
        if (!b->left_) {
            if (b->second_block_ != b) {
                b->first_block_->left_ = true;
                b->first_occurrence_level_index_ = b->first_block_->level_index_;
                if (b->second_block_ != nullptr) {
                    b->second_block_->left_ = true;
                }
                else {
                    b->second_block_ = nullptr;
                }
            } else {
                b->left_ = true;
            }
        }
    }


    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        PBlock* b = level[i];
        if (!b->left_ && b->first_block_ != b) {
            PBackBlock* bb = new PBackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->leaf_length_, input_,
                                            b->child_number_, level[b->first_occurrence_level_index_], (b->second_block_ ==
                                                                                                        nullptr) ? nullptr : level[b->first_occurrence_level_index_ +1], b->offset_);
            bb->first_block_->pointing_to_me_--;
            if (bb->second_block_ != nullptr) bb->second_block_->pointing_to_me_--;
            bb->level_index_ = b->level_index_;
            bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
            bb->left_ = b->left_;
            b->parent_->put_child(b->child_number_, bb); // Todo: parent is always InternalBlock
            level[i] = bb;
        }
    }

}

void PBlockTree::process_real_back_front_pointers() {
    std::vector<PBlock*> current_level = {root_block_};
    std::stack<PBlock*> none_blocks;
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||  current_level[0]->length() <= leaf_length_) break;
        while (current_level.size() != 0 && current_level.back()->end_index_ >= input_.size()) {
            none_blocks.push(current_level.back());
            current_level.pop_back();
        }
        process_level_real_back_front(current_level);
        while (!none_blocks.empty()) {
            current_level.push_back(none_blocks.top());
            none_blocks.pop();
        }
    }
}

void PBlockTree::process_back_front_pointers() {
    std::vector<PBlock*> current_level = {root_block_};
    std::stack<PBlock*> none_blocks;
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||  current_level[0]->length() <= leaf_length_) break;
        while (current_level.size() != 0 && current_level.back()->end_index_ >= input_.size()) {
            none_blocks.push(current_level.back());
            current_level.pop_back();
        }
        process_level_back_front(current_level);
        while (!none_blocks.empty()) {
            current_level.push_back(none_blocks.top());
            none_blocks.pop();
        }
    }
}

void PBlockTree::process_back_pointers_real_conservative_heuristic() {
    std::vector<PBlock*> current_level = {root_block_};
    std::stack<PBlock*> none_blocks;
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||  current_level[0]->length() <= leaf_length_) break;
        while (current_level.size() != 0 && current_level.back()->end_index_ >= input_.size()) {
            none_blocks.push(current_level.back());
            current_level.pop_back();
        }
        process_level_real_conservative_heuristic(current_level);
        while (!none_blocks.empty()) {
            current_level.push_back(none_blocks.top());
            none_blocks.pop();
        }
    }
}

void PBlockTree::forward_window_block_scan_real_back_and_front(std::vector<PBlock*>& level, int window_size, int N, std::unordered_map<HashString, std::vector<PBlock*>>& hashtable) {
    int i = 0;
    for (std::vector<PBlock *>::iterator it = level.begin(); it != level.end();) {
        PBlock *b = (*it);
        int offset = 0;
        RabinKarp rk(input_, (*it)->start_index_ + offset, window_size, N);
        for (; it != level.end() && ((*it) == b || (*(it-1))->end_index_ == (*it)->start_index_ - 1); it++, i++) {
            PBlock* current = *(it);
            bool last_block = ((it+1) == level.end() ||  current->end_index_ != (*(it+1))->start_index_ - 1);
            for (offset = 0; offset < current->length(); ++offset) {
                if (last_block && current->length() - offset < window_size)  break;
                HashString hS(rk.hash(), input_, current->start_index_ + offset, current->start_index_ + offset + window_size - 1);
                std::unordered_map<HashString, std::vector<PBlock *>>::const_iterator result = hashtable.find(hS);
                if (result != hashtable.end()) {
                    std::vector<PBlock*> blocks = result->second;
                    std::vector<PBlock*> new_blocks = {};
                    for (PBlock* b : blocks) {
                        if (b == current) new_blocks.push_back(b);
                        else if (!b->left_) {
                            if (offset + window_size > b->first_block_->length() && (*(it+1)) == b) {
                                new_blocks.push_back(b);
                            } else {
                                current->left_ = true;
                                b->first_occurrence_level_index_ = i;
                                b->first_block_ = current;
                                b->offset_ = offset;
                                if (offset + window_size > b->first_block_->length()) {
                                    (*(it + 1))->left_ = true;
                                    b->second_block_ = (*(it + 1));
                                }
                                else {
                                    b->second_block_ = nullptr;
                                }
                            }
                        }
                    }
                    if (new_blocks.size() == 0) hashtable.erase(hS);
                    else hashtable[hS] = new_blocks;
                }
                if (current->start_index_+offset+window_size < input_.size()) rk.next();
            }
        }
    }
}

void PBlockTree::forward_window_block_scan_real_conservative_heuristic(std::vector<PBlock*>& level, int window_size, int N, std::unordered_map<HashString, std::vector<PBlock*>>& hashtable) {
    int i = 0;
    for (std::vector<PBlock *>::iterator it = level.begin(); it != level.end();) {
        PBlock *b = (*it);
        int offset = 0;
        RabinKarp rk(input_, (*it)->start_index_ + offset, window_size, N);
        for (; it != level.end() && ((*it) == b || (*(it-1))->end_index_ == (*it)->start_index_ - 1); it++, i++) {
            PBlock* current = *(it);
            bool last_block = ((it+1) == level.end() ||  current->end_index_ != (*(it+1))->start_index_ - 1);
            for (offset = 0; offset < current->length(); ++offset) {
                if (last_block && current->length() - offset < window_size)  break;
                HashString hS(rk.hash(), input_, current->start_index_ + offset, current->start_index_ + offset + window_size - 1);
                std::unordered_map<HashString, std::vector<PBlock *>>::const_iterator result = hashtable.find(hS);
                if (result != hashtable.end()) {
                    std::vector<PBlock*> blocks = result->second;
                    std::vector<PBlock*> new_blocks = {};
                    for (PBlock* b : blocks) {
                        if (!b->left_) {
                            if (offset + window_size > b->first_block_->length() && (*(it+1)) == b) {
                                new_blocks.push_back(b);
                            } else {
                                current->left_ = true;
                                b->first_occurrence_level_index_ = i;
                                b->first_block_ = current;
                                b->offset_ = offset;
                                if (offset + window_size > b->first_block_->length()) {
                                    (*(it + 1))->left_ = true;
                                    b->second_block_ = (*(it + 1));
                                }
                                else {
                                    b->second_block_ = nullptr;
                                }
                            }
                        }
                    }
                    if (new_blocks.size() == 0) hashtable.erase(hS);
                    else hashtable[hS] = new_blocks;
                }
                if (current->start_index_+offset+window_size < input_.size()) rk.next();
            }
        }
    }
}

void PBlockTree::process_level_real_other_back_front(std::vector<PBlock*>& level) {

    int N = 6700417; // Large prime
    int level_length = level.front()->length();

    // Block scan
    std::unordered_map<HashString, std::vector<PBlock*>> hashtable;
    block_scan(level, N, hashtable);

    // Window block scan
    // This is almost the same as forward_window_block_scan, as well as the BackBlock creation
    forward_window_block_scan_real_other_back_and_front(level, level_length, N, hashtable);

    /*
    clean_self_references(level);
    update_pointing_to_me(level);
    for (auto it = level.begin(); it != level.end(); it++) {
        PBlock *current = *(it);
        if (current->first_occurrence_level_index_ > current->level_index_) {
            current->first_block_ = current;
            current->second_block_ = nullptr;
            current->first_occurrence_level_index_ = current->level_index_;
        }
        if (current->first_block_ != current) {
            current->first_block_->pointing_to_me_++;
        }
        if (current->second_block_ != nullptr && current->second_block_ != current) {
            current->second_block_->pointing_to_me_++;
        }
    }
     */

    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        PBlock* b = level[i];
        if (b->right_) {
            PBackBlock* bb = new PBackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->leaf_length_, input_,
                                            b->child_number_, level[b->first_occurrence_level_index_], (b->second_block_ ==
                                                                                                        nullptr) ? nullptr : level[b->first_occurrence_level_index_ +1], b->offset_);
            bb->first_block_->pointing_to_me_--;
            if (bb->second_block_ != nullptr) bb->second_block_->pointing_to_me_--;
            bb->level_index_ = b->level_index_;
            bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
            bb->left_ = b->left_;
            bb->right_ = b->right_;
            b->parent_->put_child(b->child_number_, bb); // Todo: parent is always InternalBlock
            level[i] = bb;
        }
    }

}

void PBlockTree::process_level_other_back_front(std::vector<PBlock*>& level) {

    int N = 6700417; // Large prime
    int level_length = level.front()->length();

    // Block scan
    std::unordered_map<HashString, std::vector<PBlock*>> hashtable;
    block_scan(level, N, hashtable);

    // Window block scan
    // This is almost the same as forward_window_block_scan, as well as the BackBlock creation
    forward_window_block_scan_front(level, level_length, N, hashtable);

    for (int i = level.size()-1; i >= 0; --i) { // NOTE REVERSE HERE
        PBlock* b = level[i];
        if (b != b->first_block_ && (b->second_block_ == nullptr || b!= b->second_block_)) {
            if (!b->left_) {
                if (/*b->first_block_->start_index_ < b->start_index_ &&*/ !b->first_block_->right_ &&
                                                                   (b->second_block_ == nullptr || !b->second_block_->right_)) {
                    b->right_ = true;
                    b->first_block_->left_ = true;
                    b->first_occurrence_level_index_ = b->first_block_->level_index_;
                    if (b->second_block_ != nullptr) {
                        b->second_block_->left_ = true;
                    }
                } else {
                    b->first_block_ = b;
                    b->second_block_ = nullptr;
                    b->offset_ = 0;
                    b->first_occurrence_level_index_ = i;
                }
            }else {
                b->first_block_ = b;
                b->second_block_ = nullptr;
                b->offset_ = 0;
                b->first_occurrence_level_index_ = i;
            }
        }else {
            b->first_block_ = b;
            b->second_block_ = nullptr;
            b->offset_ = 0;
            b->first_occurrence_level_index_ = i;
        }

    }



    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        PBlock* b = level[i];
        if (b->right_) {
            PBackBlock* bb = new PBackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->leaf_length_, input_,
                                            b->child_number_, level[b->first_occurrence_level_index_], (b->second_block_ ==
                                                                                                        nullptr) ? nullptr : level[b->first_occurrence_level_index_ +1], b->offset_);
            bb->first_block_->pointing_to_me_--;
            if (bb->second_block_ != nullptr) bb->second_block_->pointing_to_me_--;
            bb->level_index_ = b->level_index_;
            bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
            bb->left_ = b->left_;
            bb->right_ = b->right_;
            b->parent_->put_child(b->child_number_, bb); // Todo: parent is always InternalBlock
            level[i] = bb;
        }
    }

}

void PBlockTree::process_real_other_back_front_pointers() {
    std::vector<PBlock*> current_level = {root_block_};
    std::stack<PBlock*> none_blocks;
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||  current_level[0]->length() <= leaf_length_) break;
        while (current_level.size() != 0 && current_level.back()->end_index_ >= input_.size()) {
            none_blocks.push(current_level.back());
            current_level.pop_back();
        }
        process_level_real_other_back_front(current_level);
        while (!none_blocks.empty()) {
            current_level.push_back(none_blocks.top());
            none_blocks.pop();
        }
    }
}

void PBlockTree::process_other_back_front_pointers() {
    std::vector<PBlock*> current_level = {root_block_};
    std::stack<PBlock*> none_blocks;
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||  current_level[0]->length() <= leaf_length_) break;
        while (current_level.size() != 0 && current_level.back()->end_index_ >= input_.size()) {
            none_blocks.push(current_level.back());
            current_level.pop_back();
        }
        process_level_other_back_front(current_level);
        while (!none_blocks.empty()) {
            current_level.push_back(none_blocks.top());
            none_blocks.pop();
        }
    }
}

void PBlockTree::process_level_real_other_conservative_heuristic(std::vector<PBlock*>& level) {

    int N = 6700417; // Large prime
    int level_length = level.front()->length();

    // Block scan
    std::unordered_map<HashString, std::vector<PBlock*>> hashtable;
    block_scan(level, N, hashtable);

    // Window block scan
    // This is almost the same as forward_window_block_scan, as well as the BackBlock creation
    forward_window_block_scan_real_other_conservative_heuristic(level, level_length, N, hashtable);

    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        PBlock* b = level[i];
        if (b->right_) {
            PBackBlock* bb = new PBackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->leaf_length_, input_,
                                            b->child_number_, level[b->first_occurrence_level_index_], (b->second_block_ ==
                                                                                                        nullptr) ? nullptr : level[b->first_occurrence_level_index_ +1], b->offset_);
            bb->first_block_->pointing_to_me_--;
            if (bb->second_block_ != nullptr) bb->second_block_->pointing_to_me_--;
            bb->level_index_ = b->level_index_;
            bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
            bb->left_ = b->left_;
            bb->right_ = b->right_;
            b->parent_->put_child(b->child_number_, bb); // Todo: parent is always InternalBlock
            level[i] = bb;
        }
    }

}

void PBlockTree::process_level_other_conservative_heuristic(std::vector<PBlock*>& level) {

    int N = 6700417; // Large prime
    int level_length = level.front()->length();

    // Block scan
    std::unordered_map<HashString, std::vector<PBlock*>> hashtable;
    block_scan(level, N, hashtable);

    // Window block scan
    // This is almost the same as forward_window_block_scan, as well as the BackBlock creation
    forward_window_block_scan(level, level_length, N, hashtable);

    for (int i = 0; i < level.size(); ++i) {
        PBlock* b = level[i];
        if (b != b->first_block_ && (b->second_block_ == nullptr || b!= b->second_block_)) {
            if (!b->left_) {
                if (!b->first_block_->right_ && (b->second_block_ == nullptr || !b->second_block_->right_)) {
                    b->right_ = true;
                    b->first_block_->left_ = true;
                    b->first_occurrence_level_index_ = b->first_block_->level_index_;
                    if (b->second_block_ != nullptr) {
                        b->second_block_->left_ = true;
                    }
                } else {
                    b->first_block_ = b;
                    b->second_block_ = nullptr;
                    b->offset_ = 0;
                    b->first_occurrence_level_index_ = i;
                }
            }else {
                b->first_block_ = b;
                b->second_block_ = nullptr;
                b->offset_ = 0;
                b->first_occurrence_level_index_ = i;
            }
        }else {
            b->first_block_ = b;
            b->second_block_ = nullptr;
            b->offset_ = 0;
            b->first_occurrence_level_index_ = i;
        }

    }

    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        PBlock* b = level[i];
        if (b->right_) {
            PBackBlock* bb = new PBackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->leaf_length_, input_,
                                            b->child_number_, level[b->first_occurrence_level_index_], (b->second_block_ ==
                                                                                                        nullptr) ? nullptr : level[b->first_occurrence_level_index_ +1], b->offset_);
            bb->first_block_->pointing_to_me_--;
            if (bb->second_block_ != nullptr) bb->second_block_->pointing_to_me_--;
            bb->level_index_ = b->level_index_;
            bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
            bb->left_ = b->left_;
            bb->right_ = b->right_;
            b->parent_->put_child(b->child_number_, bb); // Todo: parent is always InternalBlock
            level[i] = bb;
        }
    }

}

void PBlockTree::process_back_pointers_real_other_conservative_heuristic() {
    std::vector<PBlock*> current_level = {root_block_};
    std::stack<PBlock*> none_blocks;
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||  current_level[0]->length() <= leaf_length_) break;
        while (current_level.size() != 0 && current_level.back()->end_index_ >= input_.size()) {
            none_blocks.push(current_level.back());
            current_level.pop_back();
        }
        process_level_real_other_conservative_heuristic(current_level);
        while (!none_blocks.empty()) {
            current_level.push_back(none_blocks.top());
            none_blocks.pop();
        }
    }
}

void PBlockTree::process_back_pointers_other_conservative_heuristic() {
    std::vector<PBlock*> current_level = {root_block_};
    std::stack<PBlock*> none_blocks;
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||  current_level[0]->length() <= leaf_length_) break;
        while (current_level.size() != 0 && current_level.back()->end_index_ >= input_.size()) {
            none_blocks.push(current_level.back());
            current_level.pop_back();
        }
        process_level_other_conservative_heuristic(current_level);
        while (!none_blocks.empty()) {
            current_level.push_back(none_blocks.top());
            none_blocks.pop();
        }
    }
}

void PBlockTree::process_back_pointers() {
    std::vector<PBlock*> current_level = {root_block_};
    std::stack<PBlock*> none_blocks;
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||  current_level[0]->length() <= leaf_length_) break;
        while (current_level.size() != 0 && current_level.back()->end_index_ >= input_.size()) {
            none_blocks.push(current_level.back());
            current_level.pop_back();
        }
        process_level(current_level);
        while (!none_blocks.empty()) {
            current_level.push_back(none_blocks.top());
            none_blocks.pop();
        }
    }
}


void PBlockTree::forward_window_block_scan_heuristic_concatenate(std::vector<PBlock*>& level, int window_size, int N, std::unordered_map<NonConsecutiveHashString, std::vector<PBlock*>>& hashtable) {
    int i = 0;
    NonConsecutiveRabinKarp rk(input_, level[0]->start_index_, window_size, N);
    for (std::vector<PBlock *>::iterator it = level.begin(); it != level.end(); it++, i++) {
        PBlock* current = *(it);
        for (int offset = 0; offset < current->length(); ++offset) {
            if ((it+1) == level.end() && current->length() - offset < window_size)  break;
            NonConsecutiveHashString hS(rk.hash(), current->start_index_, ((it+1) == level.end()) ? -1 : (*(it+1))->start_index_ , current->length(), input_, offset, window_size);
            std::unordered_map<NonConsecutiveHashString, std::vector<PBlock *>>::const_iterator result = hashtable.find(hS);
            if (result != hashtable.end()) {
                std::vector<PBlock*> blocks = result->second;
                for (PBlock* b : blocks) {
                    b->first_occurrence_level_index_ = i;
                    b->first_block_ = current;
                    b->offset_ = offset;
                    if (offset + window_size > b->first_block_->length()) b->second_block_ = (*(it+1));
                }
                hashtable.erase(hS);
            }

            int next_index = offset + window_size;
            if (next_index >= current->length()) {
                if ((it + 1) != level.end()) {
                    if (next_index >= current->length() + (*(it+1))->length()) {
                        if ((it + 2) != level.end()) rk.put(input_[(*(it + 2))->start_index_ + (next_index - current->length() - (*(it+1))->length())]);
                    }
                    else rk.put(input_[(*(it + 1))->start_index_ + (next_index - current->length())]);
                }
            }
            else
                rk.put(input_[current->start_index_ + next_index]);
        }
    }
}

void PBlockTree::clean_self_references(std::vector<PBlock*>& level) {
    for (auto it = level.begin(); it != level.end(); it++) {
        PBlock *current = *(it);
        if (current->second_block_ == current || current->first_block_ == current) {
            current->first_occurrence_level_index_ = current->level_index_;
            current->first_block_ = current;
            current->second_block_ = nullptr;
            current->offset_ = 0;
        }
    }
}

void PBlockTree::update_pointing_to_me(std::vector<PBlock *> & level) {
    for (auto it = level.begin(); it != level.end(); it++) {
        PBlock *current = *(it);
        if (current->first_block_ != current) {
            current->first_block_->pointing_to_me_++;
        }
        if (current->second_block_ != nullptr && current->second_block_ != current) {
            current->second_block_->pointing_to_me_++;
        }
    }
}

void PBlockTree::process_level_conservative_optimized_heuristic(std::vector<PBlock*>& level) {

    int N = 6700417; // Large prime
    int level_length = level.front()->length();

    // Block scan
    std::unordered_map<HashString, std::vector<PBlock*>> hashtable;
    block_scan(level, N, hashtable);

    // Window block pre scan
    forward_window_block_scan(level, level_length, N, hashtable);


    clean_self_references(level);
    update_pointing_to_me(level);


    // Greedy decisions
    for (PBlock* b : level) {
        if (b->first_block_ != b) {
            if (b->second_block_ == nullptr) {
                if (b->first_block_->first_block_ != b->first_block_) {
                    b->first_block_->first_block_->pointing_to_me_--;
                    if (b->first_block_->second_block_ != nullptr)
                        b->first_block_->second_block_->pointing_to_me_--;

                    b->first_block_->first_occurrence_level_index_ = b->first_block_->level_index_;
                    b->first_block_->first_block_ = b->first_block_;
                    b->first_block_->second_block_ = nullptr;
                }
            } else {
                if (b->first_block_->first_block_ != b->first_block_ && b->second_block_->first_block_ != b->second_block_) {
                    if (b->first_block_->pointing_to_me_ + b->second_block_->pointing_to_me_ > 2) {
                        b->first_block_->first_block_->pointing_to_me_--;
                        if (b->first_block_->second_block_ != nullptr)
                            b->first_block_->second_block_->pointing_to_me_--;
                        b->second_block_->first_block_->pointing_to_me_--;
                        if (b->second_block_->second_block_ != nullptr)
                            b->second_block_->second_block_->pointing_to_me_--;

                        b->first_block_->first_occurrence_level_index_ = b->first_block_->level_index_;
                        b->first_block_->first_block_ = b->first_block_;
                        b->first_block_->second_block_ = nullptr;
                        b->second_block_->first_occurrence_level_index_ = b->second_block_->level_index_;
                        b->second_block_->first_block_ = b->second_block_;
                        b->second_block_->second_block_ = nullptr;
                    } else {
                        b->first_block_->pointing_to_me_--;
                        b->second_block_->pointing_to_me_--;

                        b->first_block_ = b;
                        b->first_occurrence_level_index_ = b->level_index_;
                        b->second_block_ = nullptr;
                    }
                } else if (b->first_block_->first_block_ != b->first_block_) {
                    b->first_block_->first_block_->pointing_to_me_--;
                    if (b->first_block_->second_block_ != nullptr)
                        b->first_block_->second_block_->pointing_to_me_--;

                    b->first_block_->first_occurrence_level_index_ = b->first_block_->level_index_;
                    b->first_block_->first_block_ = b->first_block_;
                    b->first_block_->second_block_ = nullptr;

                } else if (b->second_block_->first_block_ != b->second_block_) {
                    b->second_block_->first_block_->pointing_to_me_--;
                    if (b->second_block_->second_block_ != nullptr)
                        b->second_block_->second_block_->pointing_to_me_--;

                    b->second_block_->first_occurrence_level_index_ = b->second_block_->level_index_;
                    b->second_block_->first_block_ = b->second_block_;
                    b->second_block_->second_block_ = nullptr;
                }
            }
        }
    }



    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        PBlock* b = level[i];
        if (b->first_occurrence_level_index_ < b->level_index_) {

            PBackBlock* bb = new PBackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->leaf_length_, input_,
                                          b->child_number_, level[b->first_occurrence_level_index_], (b->second_block_ ==
                                                                                                      nullptr) ? nullptr : level[b->first_occurrence_level_index_ +1], b->offset_);
            bb->level_index_ = b->level_index_;
            bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
            b->parent_->put_child(b->child_number_, bb); // Todo: parent is always InternalBlock
            level[i] = bb;
        }
    }

}


void PBlockTree::process_level_real_conservative_optimized_heuristic(std::vector<PBlock*>& level) {

    int N = 6700417; // Large prime
    int level_length = level.front()->length();

    // Block scan
    std::unordered_map<HashString, std::vector<PBlock*>> hashtable;
    block_scan(level, N, hashtable);

    // Window block pre scan
    forward_window_block_scan_real_conservative_optimized_heuristic(level, level_length, N, hashtable);


    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        PBlock* b = level[i];
        if (b->first_occurrence_level_index_ < b->level_index_) {

            PBackBlock* bb = new PBackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->leaf_length_, input_,
                                            b->child_number_, level[b->first_occurrence_level_index_], (b->second_block_ ==
                                                                                                        nullptr) ? nullptr : level[b->first_occurrence_level_index_ +1], b->offset_);
            bb->level_index_ = b->level_index_;
            bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
            b->parent_->put_child(b->child_number_, bb); // Todo: parent is always InternalBlock
            level[i] = bb;
        }
    }

}

void PBlockTree::process_level_conservative_heuristic(std::vector<PBlock*>& level) {

    int N = 6700417; // Large prime
    int level_length = level.front()->length();

    // Block scan
    std::unordered_map<HashString, std::vector<PBlock*>> hashtable;
    block_scan(level, N, hashtable);

    // Window block scan
    forward_window_block_scan(level, level_length, N, hashtable);

    clean_self_references(level);
    update_pointing_to_me(level);

    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        PBlock* b = level[i];
        if (b->pointing_to_me_ == 0 && b->first_occurrence_level_index_ < b->level_index_) {

            PBackBlock* bb = new PBackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->leaf_length_, input_,
                                            b->child_number_, level[b->first_occurrence_level_index_], (b->second_block_ ==
                                                                                                        nullptr) ? nullptr : level[b->first_occurrence_level_index_ +1], b->offset_);
            bb->level_index_ = b->level_index_;
            bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
            b->parent_->put_child(b->child_number_, bb); // Todo: parent is always InternalBlock
            level[i] = bb;
        }
    }
}


void PBlockTree::process_level_conservative_heuristic(std::vector<PBlock*>& level, int c) {

    int N = 6700417; // Large prime
    int level_length = level.front()->length();

    // Block scan
    std::unordered_map<HashString, std::vector<PBlock*>> hashtable;
    block_scan(level, N, hashtable);

    // Window block scans
    forward_window_block_scan(level, level_length, N, hashtable);
    clean_self_references(level);


    // Color cut policy
    for (PBlock* b : level) {
        if (b->first_block_ != b) {
            b->color_ = b->first_block_->color_+1;
            if (b->second_block_ != nullptr) {
                if (b->first_block_->color_ < b->second_block_->color_)
                    b->color_ = b->second_block_->color_+1;
            }
            if (b->color_ > c) {
                b->color_ = 0;
                b->first_occurrence_level_index_ = b->level_index_;
                b->first_block_ = b;
                b->second_block_ = nullptr;
                b->offset_ = 0;
            }
        }
    }

    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        PBlock* b = level[i];
        if (b->color_ > 0) {
            if (b->first_occurrence_level_index_ < b->level_index_) {
                PBackBlock *bb = new PBackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->leaf_length_,
                                              input_,
                                              b->child_number_, level[b->first_occurrence_level_index_],
                                              (b->second_block_ ==
                                               nullptr) ? nullptr : level[b->first_occurrence_level_index_ + 1],
                                              b->offset_);
                bb->level_index_ = b->level_index_;
                bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
                bb->color_ = b->color_;
                b->parent_->put_child(b->child_number_, bb); // Todo: parent is always InternalBlock

                level[i] = bb;
            }
        }
    }


}

void PBlockTree::process_level_real_conservative_heuristic(std::vector<PBlock*>& level, int c) {

    int N = 6700417; // Large prime
    int level_length = level.front()->length();

    // Block scan
    std::unordered_map<HashString, std::vector<PBlock*>> hashtable;
    block_scan(level, N, hashtable);

    // Window block scans
    forward_window_block_scan_real_conservative_heuristic(level, level_length, N, hashtable,c);

    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        PBlock* b = level[i];
        if (b->color_ > 0) {
            if (b->first_occurrence_level_index_ < b->level_index_) {
                PBackBlock *bb = new PBackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->leaf_length_,
                                                input_,
                                                b->child_number_, level[b->first_occurrence_level_index_],
                                                (b->second_block_ ==
                                                 nullptr) ? nullptr : level[b->first_occurrence_level_index_ + 1],
                                                b->offset_);
                bb->level_index_ = b->level_index_;
                bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
                bb->color_ = b->color_;
                b->parent_->put_child(b->child_number_, bb); // Todo: parent is always InternalBlock

                level[i] = bb;
            }
        }
    }
}

void PBlockTree::process_level_reverse_conservative_heuristic(std::vector<PBlock*>& level, int c) {

    int N = 6700417; // Large prime
    int level_length = level.front()->length();

    // Block scan
    std::unordered_map<HashString, std::vector<PBlock*>> hashtable;
    block_scan(level, N, hashtable);

    // Window block scans
    forward_window_block_scan(level, level_length, N, hashtable);
    clean_self_references(level);


    // Color cut reverse policy
    for (std::vector<PBlock *>::reverse_iterator rit = level.rbegin(); rit != level.rend(); rit++) {
        PBlock* b = *rit;
        if (b->first_block_ != b) {
            if (b->color_ < c) {
                if (b->color_+1 > b->first_block_->color_) {
                    b->first_block_->color_ = b->color_+1;
                }
                if (b->second_block_ != nullptr) {
                    if (b->color_+1 > b->second_block_->color_) {
                        b->second_block_->color_ = b->color_+1;
                    }
                }
            } else {
                b->first_occurrence_level_index_ = b->level_index_;
                b->first_block_ = b;
                b->second_block_ = nullptr;
                b->offset_ = 0;
            }
        }
    }

    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        PBlock* b = level[i];
        if (b->color_ < c) {
            if (b->first_occurrence_level_index_ < b->level_index_) {
                PBackBlock *bb = new PBackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->leaf_length_,
                                                input_,
                                                b->child_number_, level[b->first_occurrence_level_index_],
                                                (b->second_block_ ==
                                                 nullptr) ? nullptr : level[b->first_occurrence_level_index_ + 1],
                                                b->offset_);
                bb->level_index_ = b->level_index_;
                bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
                bb->color_ = b->color_;
                b->parent_->put_child(b->child_number_, bb); // Todo: parent is always InternalBlock

                level[i] = bb;
            }
        }
    }


}


void PBlockTree::process_level_liberal_heuristic(std::vector<PBlock*>& level) {

    int N = 6700417; // Large prime
    int level_length = level.front()->length();

    // Block scan
    std::unordered_map<HashString, std::vector<PBlock*>> hashtable;
    block_scan(level, N, hashtable);

    // Window block scans
    forward_window_block_scan(level, level_length, N, hashtable);

    clean_self_references(level);


    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        PBlock* b = level[i];
        if (b->first_occurrence_level_index_ < b->level_index_) {

            PBackBlock* bb = new PBackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->leaf_length_, input_,
                                          b->child_number_, level[b->first_occurrence_level_index_], (b->second_block_ ==
                                                                                                      nullptr) ? nullptr : level[b->first_occurrence_level_index_ +1], b->offset_);
            bb->level_index_ = b->level_index_;
            bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
            b->parent_->put_child(b->child_number_, bb); // Todo: parent is always InternalBlock
            level[i] = bb;
        }
    }

}

void PBlockTree::process_level_heuristic_concatenate(std::vector<PBlock*>& level) {

    int N = 6700417; // Large prime
    int level_length = level.front()->length();

    // Block scan
    std::unordered_map<NonConsecutiveHashString, std::vector<PBlock*>> hashtable;
    for (int i = 0; i < level.size(); ++i) {
        PBlock* b = level[i];
        NonConsecutiveRabinKarp rk(input_, b->start_index_, b->length(), N);
        NonConsecutiveHashString hS(rk.hash(),  b->start_index_, -1, b->length(), input_, 0, b->length());

        std::unordered_map<NonConsecutiveHashString, std::vector<PBlock*>>::const_iterator result = hashtable.find(hS);
    if (result == hashtable.end())
            hashtable[hS] = {b};
        else
            hashtable[hS].push_back(b);

    }


    // Window block scan
    forward_window_block_scan_heuristic_concatenate(level, level_length, N, hashtable);



    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        PBlock* b = level[i];
        if (b->first_occurrence_level_index_ < b->level_index_) {

            PBackBlock* bb = new PBackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->leaf_length_, input_,
                                          b->child_number_, level[b->first_occurrence_level_index_], (b->second_block_ ==
                                                                                                      nullptr) ? nullptr : level[b->first_occurrence_level_index_ +1], b->offset_);
            bb->level_index_ = b->level_index_;
            bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
            b->parent_->put_child(b->child_number_, bb); // Todo: parent is always InternalBlock
            level[i] = bb;
        }
    }

}

void PBlockTree::process_level_heuristic(std::vector<PBlock*>& level) {

    int N = 6700417; // Large prime
    int level_length = level.front()->length();

    // Block scan
    std::unordered_map<HashString, std::vector<PBlock*>> hashtable;
    block_scan(level, N, hashtable);

    // Window block scan
    // This is almost the same as forward_window_block_scan, as well as the BackBlock creation
    forward_window_block_scan(level, level_length, N, hashtable);



    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        PBlock* b = level[i];
        if (b->first_occurrence_level_index_ < b->level_index_) {

            PBackBlock* bb = new PBackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->leaf_length_, input_,
                                          b->child_number_, level[b->first_occurrence_level_index_], (b->second_block_ ==
                                                                                                      nullptr) ? nullptr : level[b->first_occurrence_level_index_ +1], b->offset_);
            bb->level_index_ = b->level_index_;
            bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
            b->parent_->put_child(b->child_number_, bb); // Todo: parent is always InternalBlock
            level[i] = bb;
        }
    }

}

void PBlockTree::process_back_pointers_conservative_optimized_heuristic() {
    std::vector<PBlock *> current_level = {root_block_};

    std::stack<PBlock*> none_blocks;
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||
            current_level[0]->length() <= leaf_length_)
            break;

        while (current_level.size() != 0 && current_level.back()->end_index_ >= input_.size()) {
            none_blocks.push(current_level.back());
            current_level.pop_back();
        }
        process_level_conservative_optimized_heuristic(current_level);

        while (!none_blocks.empty()) {
            current_level.push_back(none_blocks.top());
            none_blocks.pop();
        }
    }
}

void PBlockTree::process_back_pointers_real_conservative_optimized_heuristic() {
    std::vector<PBlock *> current_level = {root_block_};

    std::stack<PBlock*> none_blocks;
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||
            current_level[0]->length() <= leaf_length_)
            break;

        while (current_level.size() != 0 && current_level.back()->end_index_ >= input_.size()) {
            none_blocks.push(current_level.back());
            current_level.pop_back();
        }
        process_level_real_conservative_optimized_heuristic(current_level);

        while (!none_blocks.empty()) {
            current_level.push_back(none_blocks.top());
            none_blocks.pop();
        }
    }
}

void PBlockTree::process_back_pointers_conservative_heuristic() {
    std::vector<PBlock *> current_level = {root_block_};
    std::stack<PBlock*> none_blocks;
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||
            current_level[0]->length() <= leaf_length_)
            break;
        while (current_level.size() != 0 && current_level.back()->end_index_ >= input_.size()) {
            none_blocks.push(current_level.back());
            current_level.pop_back();
        }
        process_level_conservative_heuristic(current_level);
        while (!none_blocks.empty()) {
            current_level.push_back(none_blocks.top());
            none_blocks.pop();
        }
    }
}

void PBlockTree::process_back_pointers_conservative_heuristic(int c) {
    std::vector<PBlock *> current_level = {root_block_};
    std::stack<PBlock*> none_blocks;
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||
            current_level[0]->length() <= leaf_length_)
            break;
        while (current_level.size() != 0 && current_level.back()->end_index_ >= input_.size()) {
            none_blocks.push(current_level.back());
            current_level.pop_back();
        }
        process_level_conservative_heuristic(current_level, c);
        while (!none_blocks.empty()) {
            current_level.push_back(none_blocks.top());
            none_blocks.pop();
        }
    }
}

void PBlockTree::process_back_pointers_real_conservative_heuristic(int c) {
    std::vector<PBlock *> current_level = {root_block_};
    std::stack<PBlock*> none_blocks;
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||
            current_level[0]->length() <= leaf_length_)
            break;
        while (current_level.size() != 0 && current_level.back()->end_index_ >= input_.size()) {
            none_blocks.push(current_level.back());
            current_level.pop_back();
        }
        process_level_real_conservative_heuristic(current_level, c);
        while (!none_blocks.empty()) {
            current_level.push_back(none_blocks.top());
            none_blocks.pop();
        }
    }
}

void PBlockTree::process_back_pointers_reverse_conservative_heuristic(int c) {
    std::vector<PBlock *> current_level = {root_block_};
    std::stack<PBlock*> none_blocks;
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||
            current_level[0]->length() <= leaf_length_)
            break;
        while (current_level.size() != 0 && current_level.back()->end_index_ >= input_.size()) {
            none_blocks.push(current_level.back());
            current_level.pop_back();
        }
        process_level_reverse_conservative_heuristic(current_level, c);
        while (!none_blocks.empty()) {
            current_level.push_back(none_blocks.top());
            none_blocks.pop();
        }
    }
}

void PBlockTree::process_back_pointers_liberal_heuristic() {
    std::vector<PBlock *> current_level = {root_block_};
    std::stack<PBlock*> none_blocks;
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||
            current_level[0]->length() <= leaf_length_)
            break;
        while (current_level.size() != 0 && current_level.back()->end_index_ >= input_.size()) {
            none_blocks.push(current_level.back());
            current_level.pop_back();
        }
        process_level_liberal_heuristic(current_level);
        while (!none_blocks.empty()) {
            current_level.push_back(none_blocks.top());
            none_blocks.pop();
        }
    }
}

void PBlockTree::process_back_pointers_heuristic_concatenate() {
    std::vector<PBlock *> current_level = {root_block_};
    std::stack<PBlock*> none_blocks;
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||
            current_level[0]->length() <= leaf_length_)
            break;
        while (current_level.size() != 0 && current_level.back()->end_index_ >= input_.size()) {
            none_blocks.push(current_level.back());
            current_level.pop_back();
        }
        process_level_heuristic_concatenate(current_level);
        while (!none_blocks.empty()) {
            current_level.push_back(none_blocks.top());
            none_blocks.pop();
        }
    }
}

void PBlockTree::process_back_pointers_heuristic() {
    std::vector<PBlock *> current_level = {root_block_};
    std::stack<PBlock*> none_blocks;
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||
            current_level[0]->length() <= leaf_length_)
            break;
        while (current_level.size() != 0 && current_level.back()->end_index_ >= input_.size()) {
            none_blocks.push(current_level.back());
            current_level.pop_back();
        }
        process_level_heuristic(current_level);
        while (!none_blocks.empty()) {
            current_level.push_back(none_blocks.top());
            none_blocks.pop();
        }
    }
}