//
// Created by sking32 on 3/7/18.
//

#include "paper_blocktree/BlockTree.h"

#include <queue>
#include <iostream>
#include <paper_blocktree/LazyInternalBlock.h>
#include <paper_blocktree/LeafBlock.h>

#include "paper_blocktree/InternalBlock.h"


BlockTree::BlockTree(std::string& input, int r, int max_leaf_length): r_(r), input_(input), max_leaf_length_(max_leaf_length), open_(input[0]) {
    if (input.size() <= max_leaf_length_ || input_.size()<r)
        root_block_ = new LeafBlock(nullptr, 0, input_.size()-1, r, max_leaf_length, input_, 0);
    else
        root_block_ = new LazyInternalBlock(nullptr, 0, input_.size()-1, r, max_leaf_length, input_, 0, input_.size());

}

BlockTree::~BlockTree() {
    delete root_block_;
}

void BlockTree::add_rank_select_leaf_support() {
    root_block_->add_rank_select_leaf_support();
}

void BlockTree::add_rank_select_support(int c) {
    root_block_->add_rank_select_support(c);
}

int BlockTree::rank(int c, int i) {
    return root_block_->rank(c, i);
}

int BlockTree::rank_alternative(int c, int i) {
    return root_block_->rank_alternative(c, i);
}

int BlockTree::better_rank(int c, int i) {
    return root_block_->better_rank(c, i);
}

int BlockTree::select(int c, int j) {
    return root_block_->select(c, j);
}

int BlockTree::select_alternative(int c, int j) {
    return root_block_->select_alternative(c, j);
}

int BlockTree::better_select(int c, int j) {
    return root_block_->better_select(c, j);
}

int BlockTree::leaf_rank(int i) {
    return root_block_->leaf_rank(i);
}

int BlockTree::leaf_rank_alternative(int i) {
    return root_block_->leaf_rank_alternative(i);
}

int BlockTree::better_leaf_rank(int i) {
    return root_block_->better_leaf_rank(i);
}

int BlockTree::leaf_select(int j) {
    return root_block_->leaf_select(j);
}

int BlockTree::leaf_select_alternative(int j) {
    return root_block_->leaf_select_alternative(j);
}

int BlockTree::better_leaf_select(int j) {
    return root_block_->better_leaf_select(j);
}




void BlockTree::print_statistics_2() {
    std::cout << "Number of nodes : " << number_of_nodes() << std::endl;
    std::cout << "Number of leaves : " << number_of_leaves() << std::endl;
    std::cout << "Number of InternalBlocks : " << number_of_internal_blocks() << std::endl;
    std::cout << "Number of BackBlocks : " << number_of_back_blocks() << std::endl;
    std::cout << "Number of LeafBlocks : " << number_of_leaf_blocks() << std::endl;
    std::cout << "Height : " << height() << std::endl;
    std::cout << "Max Hop : " << get_max_hop() << std::endl;
    std::cout << std::endl;

    bool ok = true;
    int max = 0;
    for (int i = 0; i < input_.size(); ++i) {
        int a = 0;
        ok = ok && (access_2(i,a) == (int) input_[i]);
        max = (max > a) ? max : a;
    }
    if (!ok) std::cout << "Error" << std::endl;
    std::cout << "Max Number of Back Pointers taken : " << max << std::endl;

}

void BlockTree::print_statistics() {
    std::cout << "Number of nodes : " << number_of_nodes() << std::endl;
    std::cout << "Number of leaves : " << number_of_leaves() << std::endl;
    std::cout << "Number of InternalBlocks : " << number_of_internal_blocks() << std::endl;
    std::cout << "Number of BackBlocks : " << number_of_back_blocks() << std::endl;
    std::cout << "Number of LeafBlocks : " << number_of_leaf_blocks() << std::endl;
    std::cout << std::endl;
/*
    bool ok = true;
    for (int i = 0; i < input_.size(); ++i) {
        ok = ok && (access(i) == (int) input_[i]);
    }
    if (!ok) std::cout << "Error" << std::endl;
*/
}

std::vector<std::vector<Block*>> BlockTree::levelwise_iterator() {
    std::vector<std::vector<Block*>> result = {{root_block_}};
    while (!dynamic_cast<LeafBlock*>(result.back()[0])) {
        std::vector<Block*> next_level = {};
        for (Block *b : result.back())
            for (Block *child : b->children())
                next_level.push_back(child);
        result.push_back(next_level);
    }

    return result;
}

int BlockTree::number_of_nodes() {
    return root_block_->number_of_nodes();
}

int BlockTree::number_of_back_blocks() {
    return root_block_->number_of_back_blocks();
}

int BlockTree::number_of_internal_blocks() {
    return root_block_->number_of_internal_blocks();
}

int BlockTree::number_of_leaf_blocks() {
    return root_block_->number_of_leaf_blocks();
}

int BlockTree::height() {
    return root_block_->height();
}

int BlockTree::number_of_leaves() {
    return root_block_->number_of_leaves();
}

void BlockTree::clean_unnecessary_expansions() {
    root_block_->clean_unnecessary_expansions();
    for (std::vector<Block*> level : levelwise_iterator()) {
        for (int i = 0; i<level.size(); ++i) {
            level[i]->level_index_ = i;
            level[i]->first_occurrence_level_index_ = level[i]->first_block_->level_index_;
        }
    }
}

void BlockTree::clean_unnecessary_expansions(int c) {
    root_block_->put_fix_references();
    root_block_->clean_unnecessary_expansions(c);
    for (std::vector<Block*> level : levelwise_iterator()) {
        std::priority_queue<Block*, std::vector<Block*>, blockcomparison> q;
        for (int i = 0; i < level.size(); ++i) {
            Block* b = level[i];
            b->level_index_ = i;
            b->first_occurrence_level_index_ = i;
            if (b->fix_reference_ != -1)
                q.push(b);
        }

        int i = 0;
        while (!q.empty()) {
            Block * bb = q.top();
            q.pop();
            for (; level[i]->start_index_ < bb->fix_reference_; ++i);
            bb->first_occurrence_level_index_ = i;

            bb->first_block_ = level[i];
            if (bb->second_block_ != nullptr) bb->second_block_ = level[i+1];
        }

    }
}

int BlockTree::access(int i) {
    return root_block_->access(i);
}

int BlockTree::access_2(int i, int& a) {
    return root_block_->access_2(i,a);
}

int BlockTree::number_of_selfreferences() {
    return root_block_->number_of_selfreferences();
}

bool BlockTree::check_heuristic() {
    return root_block_->check_heuristic();
}

bool BlockTree::check() {
    return root_block_->check();
}

bool BlockTree::contains_double_pointer() {
    return root_block_->contains_double_pointer(false);
}

int BlockTree::get_max_hop() {
    return root_block_->compute_max_hop();
}


std::vector<Block*> BlockTree::next_level(std::vector<Block*>& level) {
    std::vector<Block*> next_level;
    for (int i = 0; i < level.size(); ++i) {
        Block* b = level[i];
        for (Block *child : b->children()) { // Do it in order
            child->level_index_ = next_level.size();
            child->first_occurrence_level_index_ = next_level.size();
            next_level.push_back(child);
        }
    }
    return next_level;
}

void backward_window_block_scan(std::vector<Block*>& level, int window_size, BlockTree* bt, int N, std::unordered_map<HashString, std::vector<Block*>>& hashtable) {
    //std::cout << "RUNNING WINDOW_BLOCK SCANNING for length: " << window_size << std::endl;
    for (std::vector<Block *>::reverse_iterator rit = level.rbegin(); rit != level.rend();) {
        Block *b = (*rit);
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
                Block* current = (*rit);
                HashString hS(brk->hash(), bt->input_, current->start_index_ + offset, current->start_index_ + offset + window_size - 1);
                std::unordered_map<HashString, std::vector<Block *>>::const_iterator result = hashtable.find(hS);
                if (result != hashtable.end()) {
                    std::vector<Block*> blocks = result->second;
                    for (Block* b : blocks) {
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


void backward_pair_window_block_scan(std::vector<Block*>& level, int pair_window_size, BlockTree* bt, int N, std::unordered_map<HashString, std::vector<std::pair<Block*, Block*>>>& pair_hashtable) {
    for (std::vector<Block *>::reverse_iterator rit = level.rbegin(); rit != level.rend();) {
        Block *b = (*rit);
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
                Block* current = (*rit);
                HashString hS(brk->hash(), bt->input_, current->start_index_ + offset, current->start_index_ + offset + pair_window_size - 1);
                std::unordered_map<HashString, std::vector<std::pair<Block *,Block*>>>::const_iterator result = pair_hashtable.find(hS);
                if (result != pair_hashtable.end()) {
                    for (std::pair<Block*,Block*> p: result->second) {
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

void BlockTree::forward_pair_window_block_scan(std::vector<Block*>& level, int pair_window_size, int N, std::unordered_map<HashString, std::vector<std::pair<Block*, Block*>>>& pair_hashtable) {
    for (std::vector<Block *>::iterator it = level.begin(); it != level.end();) {
        Block *b = (*it);
        b->right_ = true;
        int offset = 0;
        RabinKarp rk(input_, (*it)->start_index_ + offset, pair_window_size, N); // offset is always 0 here
        for (; it != level.end() && ((*it) == b || (*(it-1))->end_index_ == (*it)->start_index_ - 1); it++) {
            Block* current = *(it);
            bool last_block = ((it+1) == level.end() ||  current->end_index_ != (*(it+1))->start_index_ - 1);
            for (offset = 0; offset < current->length(); ++offset) {
                if (last_block && current->length() - offset < pair_window_size)  break;
                HashString hS(rk.hash(), input_, current->start_index_ + offset, current->start_index_ + offset + pair_window_size - 1);
                std::unordered_map<HashString, std::vector<std::pair<Block*, Block*>>>::const_iterator result = pair_hashtable.find(hS);
                if (result != pair_hashtable.end()) { // Here, It could be that the scanning should have finished with the penultimate, but it never should enter this ''if''
                                                        // when We're on the penultimate block and the window exceeds the last block because if that is a first occurrence should have been occured before in a pair of blocks
                                                        // maybe use a condition more like rk's condition below could work fine too
                                                        // Same logic: for when passing a window of size 2l + 2 over 2 block of length l
                    for (std::pair<Block*,Block*> p: result->second) {
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




void BlockTree::forward_window_block_scan(std::vector<Block*>& level, int window_size, int N, std::unordered_map<HashString, std::vector<Block*>>& hashtable) {
    int i = 0;
    for (std::vector<Block *>::iterator it = level.begin(); it != level.end();) {
        Block *b = (*it);
        int offset = 0;
        RabinKarp rk(input_, (*it)->start_index_ + offset, window_size, N);
        for (; it != level.end() && ((*it) == b || (*(it-1))->end_index_ == (*it)->start_index_ - 1); it++, i++) {
            Block* current = *(it);
            bool last_block = ((it+1) == level.end() ||  current->end_index_ != (*(it+1))->start_index_ - 1);
            for (offset = 0; offset < current->length(); ++offset) {
                if (last_block && current->length() - offset < window_size)  break;
                HashString hS(rk.hash(), input_, current->start_index_ + offset, current->start_index_ + offset + window_size - 1);
                std::unordered_map<HashString, std::vector<Block *>>::const_iterator result = hashtable.find(hS);
                if (result != hashtable.end()) {
                    std::vector<Block*> blocks = result->second;
                    for (Block* b : blocks) {
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

void BlockTree::forward_window_block_scan_front(std::vector<Block*>& level, int window_size, int N, std::unordered_map<HashString, std::vector<Block*>>& hashtable) {
    int i = 0;
    for (std::vector<Block *>::iterator it = level.begin(); it != level.end();) {
        Block *b = (*it);
        int offset = 0;
        RabinKarp rk(input_, (*it)->start_index_ + offset, window_size, N);
        for (; it != level.end() && ((*it) == b || (*(it-1))->end_index_ == (*it)->start_index_ - 1); it++, i++) {
            Block* current = *(it);
            bool last_block = ((it+1) == level.end() ||  current->end_index_ != (*(it+1))->start_index_ - 1);
            for (offset = 0; offset < current->length(); ++offset) {
                if (last_block && current->length() - offset < window_size)  break;
                HashString hS(rk.hash(), input_, current->start_index_ + offset, current->start_index_ + offset + window_size - 1);
                std::unordered_map<HashString, std::vector<Block *>>::const_iterator result = hashtable.find(hS);
                if (result != hashtable.end()) {
                    std::vector<Block*> blocks = result->second;
                    std::vector<Block*> new_blocks = {};
                    for (Block* b : blocks) {
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


void BlockTree::block_scan(std::vector<Block *>& level, int N , std::unordered_map<int, std::unordered_map<HashString, std::vector<Block*>>>& block_tables) {
    for (Block* b : level) {
        RabinKarp rk(input_, b->start_index_, b->length(), N);
        HashString hS(rk.hash(),  input_, b->start_index_, b->end_index_);

        std::unordered_map<int, std::unordered_map<HashString, std::vector<Block*>>>::const_iterator result_table = block_tables.find(b->length());
        if (result_table == block_tables.end()) {
            block_tables[b->length()] = {};
        }

        std::unordered_map<HashString, std::vector<Block*>>& hashtable = block_tables[b->length()];
        std::unordered_map<HashString, std::vector<Block*>>::const_iterator result = hashtable.find(hS);

        if (result == hashtable.end())
            hashtable[hS] = {b};
        else
            hashtable[hS].push_back(b);
    }
}


void BlockTree::process_level(std::vector<Block*>& level) {

    int N = 6700417; //Large prime


    // Block scan
    std::unordered_map<int, std::unordered_map<HashString, std::vector<Block*>>> block_tables;
    block_scan(level, N, block_tables);

    // Pairs of blocks scan
    std::unordered_map<int, std::unordered_map<HashString, std::vector<std::pair<Block *,Block*>>>> pair_block_tables;
    for (std::vector<Block *>::iterator it = level.begin(); it != level.end();) {
        for (++it; (it != level.end() && (*(it-1))->end_index_ == (*it)->start_index_ - 1); ++it) {
            Block* current = (*(it-1));
            Block* next = (*it);
            RabinKarp rk(input_, current->start_index_, current->length() + next->length(), N);
            HashString hS(rk.hash(), input_, current->start_index_, current->start_index_ + current->length() + next->length()-1); // Second parameter is next->end_index

            std::unordered_map<int, std::unordered_map<HashString, std::vector<std::pair<Block *,Block*>>>>::const_iterator result_table = pair_block_tables.find(current->length()+next->length());
            if (result_table == pair_block_tables.end()) {
                pair_block_tables[current->length()+next->length()] = {};
            }

            std::unordered_map<HashString, std::vector<std::pair<Block *,Block*>>>& pair_hashtable = pair_block_tables[current->length()+next->length()];
            std::unordered_map<HashString, std::vector<std::pair<Block *,Block*>>>::const_iterator result = pair_hashtable.find(hS);

            if (result == pair_hashtable.end())
                pair_hashtable[hS] = {{current, next}};
            else
                pair_hashtable[hS].push_back({current, next});
        }
    }


    // Window block scans
    for (auto it : block_tables) {
        std::unordered_map<HashString, std::vector<Block*>>& hashtable = block_tables[it.first];
        //Establishes first occurrences of blocks
        forward_window_block_scan(level, it.first, N, hashtable);
    }



    // Window Pair of blocks scans
    for (auto it : pair_block_tables) {
        std::unordered_map<HashString, std::vector<std::pair<Block *,Block*>>>& pair_hashtable = pair_block_tables[it.first];
        forward_pair_window_block_scan(level, it.first, N, pair_hashtable);
    }



    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        Block* b = level[i];
        if (b->left_ && b->right_ && b->first_occurrence_level_index_ < b->level_index_) {
            // This doesn't have the bug of the dangling reference fixed with first_occurrence_level_index, because it shouldn't happen that
            // A block points back to a BackBlock
            BackBlock* bb = new BackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->max_leaf_length_, input_,
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

void BlockTree::process_back_pointers() {
    std::vector<Block*> current_level = {root_block_};
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||  current_level[0]->length() <= max_leaf_length_) break;
        process_level(current_level);
    }
}

void BlockTree::forward_window_block_scan_heuristic_concatenate(std::vector<Block*>& level, int window_size, int N, std::unordered_map<NonConsecutiveHashString, std::vector<Block*>>& hashtable) {
    int i = 0;
    NonConsecutiveRabinKarp rk(input_, level[0]->start_index_, window_size, N);
    for (std::vector<Block *>::iterator it = level.begin(); it != level.end(); it++, i++) {
        Block* current = *(it);
        for (int offset = 0; offset < current->length(); ++offset) {
            if ((it+1) == level.end() && current->length() - offset < window_size)  break;
            NonConsecutiveHashString hS(rk.hash(), current->start_index_, ((it+1) == level.end()) ? -1 : (*(it+1))->start_index_, current->length(), input_ , offset, window_size);
            std::unordered_map<NonConsecutiveHashString, std::vector<Block *>>::const_iterator result = hashtable.find(hS);
            if (result != hashtable.end()) {
                std::vector<Block*> blocks = result->second;
                for (Block* b : blocks) {
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

void BlockTree::clean_self_references(std::vector<Block*>& level) {
    for (auto it = level.begin(); it != level.end(); it++) {
        Block *current = *(it);
        if (current->second_block_ == current|| current->first_block_ == current) {
            current->first_occurrence_level_index_ = current->level_index_;
            current->first_block_ = current;
            current->second_block_ = nullptr;
            current->offset_ = 0;
        }
    }
}

void BlockTree::update_pointing_to_me(std::vector<Block *> & level) {
    for (auto it = level.begin(); it != level.end(); it++) {
        Block *current = *(it);
        if (current->first_block_ != current) {
            current->first_block_->pointing_to_me_++;
        }
        if (current->second_block_ != nullptr && current->second_block_ != current) {
            current->second_block_->pointing_to_me_++;
        }
    }
}

void BlockTree::process_level_conservative_optimized_heuristic(std::vector<Block*>& level) {

    int N = 6700417; // Large prime

    // Block scan
    std::unordered_map<int, std::unordered_map<HashString, std::vector<Block*>>> block_tables;
    block_scan(level, N, block_tables);

    // Window block pre scans
    for (auto it : block_tables) {
        std::unordered_map<HashString, std::vector<Block*>> hashtable = block_tables[it.first];
        forward_window_block_scan(level, it.first, N, hashtable);
    }

    clean_self_references(level);
    update_pointing_to_me(level);


    // Greedy decisions
    for (Block* b : level) {
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
        Block* b = level[i];
        if (b->first_occurrence_level_index_ < b->level_index_) {

            BackBlock* bb = new BackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->max_leaf_length_, input_,
                                          b->child_number_, level[b->first_occurrence_level_index_], (b->second_block_ ==
                                                                                                      nullptr) ? nullptr : level[b->first_occurrence_level_index_ +1], b->offset_);
            bb->level_index_ = b->level_index_;
            bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
            b->parent_->put_child(b->child_number_, bb); // Todo: parent is always InternalBlock
            level[i] = bb;
        }
    }

}



void BlockTree::process_level_conservative_heuristic(std::vector<Block*>& level) {

    int N = 6700417; // Large prime

    // Block scan
    std::unordered_map<int, std::unordered_map<HashString, std::vector<Block*>>> block_tables;
    block_scan(level, N, block_tables);

    // Window block scans
    for (auto it : block_tables) {
        std::unordered_map<HashString, std::vector<Block*>>& hashtable = block_tables[it.first];
        forward_window_block_scan(level, it.first, N, hashtable);
    }
    clean_self_references(level);
    update_pointing_to_me(level);

    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        Block* b = level[i];
        if (b->pointing_to_me_ == 0 && b->first_occurrence_level_index_ < b->level_index_) {

            BackBlock* bb = new BackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->max_leaf_length_, input_,
                                          b->child_number_, level[b->first_occurrence_level_index_], (b->second_block_ ==
                                                                                                      nullptr) ? nullptr : level[b->first_occurrence_level_index_ +1], b->offset_);
            bb->level_index_ = b->level_index_;
            bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
            b->parent_->put_child(b->child_number_, bb); // Todo: parent is always InternalBlock
            level[i] = bb;
        }
    }

}

void BlockTree::process_level_back_front(std::vector<Block*>& level) {

    int N = 6700417; // Large prime

    // Block scan
    std::unordered_map<int, std::unordered_map<HashString, std::vector<Block*>>> block_tables;
    block_scan(level, N, block_tables);

    // Window block scans
    for (auto it : block_tables) {
        std::unordered_map<HashString, std::vector<Block*>>& hashtable = block_tables[it.first];
        forward_window_block_scan_front(level, it.first, N, hashtable);
    }
    for (int i = 0; i < level.size(); ++i) {
        Block *b = level[i];
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
        Block* b = level[i];
        if (!b->left_ && b->first_block_ != b) {
            BackBlock* bb = new BackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->max_leaf_length_, input_,
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


void BlockTree::process_level_other_back_front(std::vector<Block*>& level) {

    int N = 6700417; // Large prime

    // Block scan
    std::unordered_map<int, std::unordered_map<HashString, std::vector<Block*>>> block_tables;
    block_scan(level, N, block_tables);

    // Window block scans
    for (auto it : block_tables) {
        std::unordered_map<HashString, std::vector<Block*>>& hashtable = block_tables[it.first];
        forward_window_block_scan_front(level, it.first, N, hashtable);
    }
    for (int i = level.size()-1; i >= 0; --i) { // NOTE REVERSE HERE
        Block* b = level[i];
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
        Block* b = level[i];
        if (b->right_) {
            BackBlock* bb = new BackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->max_leaf_length_, input_,
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


void BlockTree::process_level_other_conservative_heuristic(std::vector<Block*>& level) {

    int N = 6700417; // Large prime

    // Block scan
    std::unordered_map<int, std::unordered_map<HashString, std::vector<Block*>>> block_tables;
    block_scan(level, N, block_tables);

    // Window block scans
    for (auto it : block_tables) {
        std::unordered_map<HashString, std::vector<Block*>>& hashtable = block_tables[it.first];
        forward_window_block_scan(level, it.first, N, hashtable);
    }
    for (int i = 0; i < level.size(); ++i) {
        Block* b = level[i];
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
        Block* b = level[i];
        if (b->right_) {
            BackBlock* bb = new BackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->max_leaf_length_, input_,
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


void BlockTree::process_level_conservative_heuristic(std::vector<Block*>& level, int c) {

    int N = 6700417; // Large prime

    // Block scan
    std::unordered_map<int, std::unordered_map<HashString, std::vector<Block*>>> block_tables;
    block_scan(level, N, block_tables);

    // Window block scans
    for (auto it : block_tables) {
        std::unordered_map<HashString, std::vector<Block*>> hashtable = block_tables[it.first];
        forward_window_block_scan(level, it.first, N, hashtable);
    }
    clean_self_references(level);


    // Color cut policy
    for (Block* b : level) {
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
        Block* b = level[i];
        if (b->color_ > 0) {
            if (b->first_occurrence_level_index_ < b->level_index_) {
                BackBlock *bb = new BackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->max_leaf_length_,
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


void BlockTree::process_level_reverse_conservative_heuristic(std::vector<Block*>& level, int c) {

    int N = 6700417; // Large prime

    // Block scan
    std::unordered_map<int, std::unordered_map<HashString, std::vector<Block*>>> block_tables;
    block_scan(level, N, block_tables);

    // Window block scans
    for (auto it : block_tables) {
        std::unordered_map<HashString, std::vector<Block*>> hashtable = block_tables[it.first];
        forward_window_block_scan(level, it.first, N, hashtable);
    }
    clean_self_references(level);


    // Color cut reverse  policy
    for (std::vector<Block *>::reverse_iterator rit = level.rbegin(); rit != level.rend(); rit++) {
        Block* b = *rit;
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
        Block* b = level[i];
        if (b->color_ < c) {
            if (b->first_occurrence_level_index_ < b->level_index_) {
                BackBlock *bb = new BackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->max_leaf_length_,
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


void BlockTree::process_level_liberal_heuristic(std::vector<Block*>& level) {

    int N = 6700417; // Large prime

    // Block scan
    std::unordered_map<int, std::unordered_map<HashString, std::vector<Block*>>> block_tables;
    block_scan(level, N, block_tables);

    // Window block scans
    for (auto it : block_tables) {
        std::unordered_map<HashString, std::vector<Block*>>& hashtable = block_tables[it.first];
        forward_window_block_scan(level, it.first, N, hashtable);
    }
    clean_self_references(level);


    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        Block* b = level[i];
        if (b->first_occurrence_level_index_ < b->level_index_) {

            BackBlock* bb = new BackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->max_leaf_length_, input_,
                                          b->child_number_, level[b->first_occurrence_level_index_], (b->second_block_ ==
                                                                                                      nullptr) ? nullptr : level[b->first_occurrence_level_index_ +1], b->offset_);
            bb->level_index_ = b->level_index_;
            bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
            b->parent_->put_child(b->child_number_, bb); // Todo: parent is always InternalBlock
            level[i] = bb;
        }
    }

}



void BlockTree::process_level_heuristic_concatenate(std::vector<Block*>& level) {

    int N = 6700417; // Large prime

    // Block scan
    std::unordered_map<int, std::unordered_map<NonConsecutiveHashString, std::vector<Block*>>> block_tables;
    for (int i = 0; i < level.size(); ++i) {
        Block* b = level[i];
        NonConsecutiveRabinKarp rk(input_, b->start_index_, b->length(), N);
        NonConsecutiveHashString hS(rk.hash(),  b->start_index_, -1, b->length(), input_, 0, b->length());

        std::unordered_map<int, std::unordered_map<NonConsecutiveHashString, std::vector<Block*>>>::const_iterator result_table = block_tables.find(b->length());
        if (result_table == block_tables.end()) {
            block_tables[b->length()] = {};
        }

        std::unordered_map<NonConsecutiveHashString, std::vector<Block*>>& hashtable = block_tables[b->length()];
        std::unordered_map<NonConsecutiveHashString, std::vector<Block*>>::const_iterator result = hashtable.find(hS);

        if (result == hashtable.end())
            hashtable[hS] = {b};
        else
            hashtable[hS].push_back(b);

    }


    // Window block scans
    for (auto it : block_tables) {
        std::unordered_map<NonConsecutiveHashString, std::vector<Block*>>& hashtable = block_tables[it.first];
        forward_window_block_scan_heuristic_concatenate(level, it.first, N, hashtable);
    }



    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        Block* b = level[i];
        if (b->first_occurrence_level_index_ < b->level_index_) {

            BackBlock* bb = new BackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->max_leaf_length_, input_,
                                          b->child_number_, level[b->first_occurrence_level_index_], (b->second_block_ ==
                                                                                                      nullptr) ? nullptr : level[b->first_occurrence_level_index_ +1], b->offset_);
            bb->level_index_ = b->level_index_;
            bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
            b->parent_->put_child(b->child_number_, bb); // Todo: parent is always InternalBlock
            level[i] = bb;
        }
    }

}


void BlockTree::process_level_heuristic(std::vector<Block*>& level) {

    int N = 6700417; // Large prime

    // Block scan
    std::unordered_map<int, std::unordered_map<HashString, std::vector<Block*>>> block_tables;
    block_scan(level, N, block_tables);

    // Window block scans
    for (auto it : block_tables) {
        std::unordered_map<HashString, std::vector<Block*>>& hashtable = block_tables[it.first];
        // This is almost the same as forward_window_block_scan, as well as the BackBlock creation
        forward_window_block_scan(level, it.first, N, hashtable);
    }



    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        Block* b = level[i];
        if (b->first_occurrence_level_index_ < b->level_index_) {

            BackBlock* bb = new BackBlock(b->parent_, b->start_index_, b->end_index_, b->r_, b->max_leaf_length_, input_,
                                          b->child_number_, level[b->first_occurrence_level_index_], (b->second_block_ ==
                                                                                                      nullptr) ? nullptr : level[b->first_occurrence_level_index_ +1], b->offset_);
            bb->level_index_ = b->level_index_;
            bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
            b->parent_->put_child(b->child_number_, bb); // Todo: parent is always InternalBlock
            level[i] = bb;
        }
    }

}



void BlockTree::process_back_pointers_conservative_optimized_heuristic() {
    std::vector<Block *> current_level = {root_block_};
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||
            current_level[0]->length() <= max_leaf_length_)
            break;
        process_level_conservative_optimized_heuristic(current_level);
    }
}

void BlockTree::process_back_pointers_conservative_heuristic() {
    std::vector<Block *> current_level = {root_block_};
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||
            current_level[0]->length() <= max_leaf_length_)
            break;
        process_level_conservative_heuristic(current_level);
    }
}

void BlockTree::process_back_front_pointers() {
    std::vector<Block *> current_level = {root_block_};
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||
            current_level[0]->length() <= max_leaf_length_)
            break;
        process_level_back_front(current_level);
    }
}

void BlockTree::process_other_back_front_pointers() {
    std::vector<Block *> current_level = {root_block_};
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||
            current_level[0]->length() <= max_leaf_length_)
            break;
        process_level_other_back_front(current_level);
    }
}

void BlockTree::process_back_pointers_other_conservative_heuristic() {
    std::vector<Block *> current_level = {root_block_};
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||
            current_level[0]->length() <= max_leaf_length_)
            break;
        process_level_other_conservative_heuristic(current_level);
    }
}

void BlockTree::process_back_pointers_conservative_heuristic(int c) {
    std::vector<Block *> current_level = {root_block_};
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||
            current_level[0]->length() <= max_leaf_length_)
            break;
        //std::cout << current_level[0]->length() << std::endl;
        process_level_conservative_heuristic(current_level, c);
    }
}


void BlockTree::process_back_pointers_reverse_conservative_heuristic(int c) {
    std::vector<Block *> current_level = {root_block_};
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||
            current_level[0]->length() <= max_leaf_length_)
            break;
        process_level_reverse_conservative_heuristic(current_level, c);
    }
}

void BlockTree::process_back_pointers_liberal_heuristic() {
    std::vector<Block *> current_level = {root_block_};
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||
            current_level[0]->length() <= max_leaf_length_)
            break;
        process_level_liberal_heuristic(current_level);
    }
}


void BlockTree::process_back_pointers_heuristic_concatenate() {
    std::vector<Block *> current_level = {root_block_};
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||
            current_level[0]->length() <= max_leaf_length_)
            break;
        process_level_heuristic_concatenate(current_level);
    }
}

void BlockTree::process_back_pointers_heuristic() {
    std::vector<Block *> current_level = {root_block_};
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||
            current_level[0]->length() <= max_leaf_length_)
            break;
        process_level_heuristic(current_level);
    }
}