//
// Created by sking32 on 5/18/18.
//

#include <compressed/PCBlockTree.h>
#include <iostream>
#include <unordered_set>

PCBlockTree::PCBlockTree(PBlockTree * bt) : r_(bt->r_), leaf_length_(bt->leaf_length_), input_(bt->input_) {
    std::vector<PBlock*> first_level = {bt->root_block_};
    bool is_first_level = false;
    while (!is_first_level) {
        for (PBlock* b: first_level) {
            is_first_level = is_first_level || b->is_leaf();
        }
        if (is_first_level) break;
        first_level = bt->next_level(first_level);
    }

    std::unordered_map<int,int> first_level_prefix_ranks_;
    std::unordered_map<int,sdsl::int_vector<>*> first_level_prefix_ranks;
    std::unordered_map<int,sdsl::int_vector<>*> first_level_cumulated_ranks;
    std::unordered_map<int,sdsl::int_vector<>*> first_level_ranks;

    for (auto pair: first_level[0]->prefix_ranks_)
        first_level_prefix_ranks_[pair.first] = 0;

    for (auto pair: first_level[0]->ranks_) { //auto &
        first_level_prefix_ranks[pair.first] = new sdsl::int_vector<>(first_level.size());
        first_level_cumulated_ranks[pair.first] = new sdsl::int_vector<>(first_level.size());
        first_level_ranks[pair.first] = new sdsl::int_vector<>(first_level.size());
    }
    for (int i = 0; i < first_level.size(); ++i) {
        for (auto pair: first_level_prefix_ranks_)
            (*first_level_prefix_ranks[pair.first])[i] = pair.second;

        for (auto pair: first_level[i]->ranks_) {
            (*first_level_ranks[pair.first])[i] = first_level[i]->ranks_[pair.first];
            first_level_prefix_ranks_[pair.first] = first_level_prefix_ranks_[pair.first] + first_level[i]->ranks_[pair.first];
        }
        for (auto pair: first_level_prefix_ranks_)
            (*first_level_cumulated_ranks[pair.first])[i] = pair.second;
    }
    for (auto pair : first_level_prefix_ranks) {
        sdsl::util::bit_compress(*(pair.second));
        bt_first_level_prefix_ranks_[pair.first] = pair.second;
        c_bt_first_level_prefix_ranks_[pair.first] = new sdsl::dac_vector_dp<sdsl::rrr_vector<>>(*(pair.second));
    }
    for (auto pair : first_level_cumulated_ranks) {
        sdsl::util::bit_compress(*(pair.second));
        bt_first_level_cumulated_ranks_[pair.first] = pair.second;
        c_bt_first_level_cumulated_ranks_[pair.first] = new sdsl::dac_vector_dp<sdsl::rrr_vector<>>(*(pair.second));
    }
    for (auto pair : first_level_ranks) {
        sdsl::util::bit_compress(*(pair.second));
        bt_ranks_[pair.first].push_back(pair.second);
        c_bt_ranks_[pair.first].push_back(new sdsl::dac_vector_dp<sdsl::rrr_vector<>>(*(pair.second)));
    }

    first_level_length_ = first_level[0]->length();
    number_of_blocks_first_level_ = first_level.size();
    number_of_levels_ = 0;

    std::vector<PBlock*> current_level = first_level;
    std::vector<PBlock*> next_level = bt->next_level(first_level);

    while (next_level.size() != 0) {
        sdsl::bit_vector* current_level_bv = new sdsl::bit_vector(current_level.size() ,0);

        std::unordered_map<int,sdsl::int_vector<>*> next_level_ranks;
        std::unordered_map<int,sdsl::int_vector<>*> next_level_prefix_ranks;
        std::unordered_map<int,sdsl::int_vector<>*> next_level_better_prefix_ranks;
        std::unordered_map<int,sdsl::int_vector<>*> next_level_cumulated_ranks;

        for (auto pair: next_level[0]->ranks_) { // Case root_block (?)
            next_level_ranks[pair.first] = new sdsl::int_vector<>(next_level.size());
            next_level_better_prefix_ranks[pair.first] = new sdsl::int_vector<>(next_level.size());
        }
        for (auto pair: next_level[0]->prefix_ranks_) { // Case root_block (?)
            next_level_prefix_ranks[pair.first] = new sdsl::int_vector<>(next_level.size());
        }
        for (auto pair: next_level[0]->cumulated_ranks_) { // Case root_block (?)
            next_level_cumulated_ranks[pair.first] = new sdsl::int_vector<>(next_level.size());
        }

        int number_of_leaves = 0;
        int current_length = current_level.front()->length();
        for (int i = 0; i < current_level.size(); ++i) {
            current_level[i]->level_index_ = i;

            if (current_level[i]->is_leaf()) {
                (*current_level_bv)[i] = 0;
                ++number_of_leaves;
            }
            else {
                (*current_level_bv)[i] = 1;
            }
        }

        for (int i = 0; i < next_level.size(); ++i) {
            if (i == 0) {
                for (auto pair: next_level[next_level.size()-1]->ranks_)
                    (*next_level_better_prefix_ranks[pair.first])[i] = pair.second;
            } else if (i%r_ == 0) {
                for (auto pair: next_level[i-1]->ranks_)
                    (*next_level_better_prefix_ranks[pair.first])[i] = pair.second;
            } else {
                for (auto pair: next_level[i]->prefix_ranks_)
                    (*next_level_better_prefix_ranks[pair.first])[i] = pair.second;
            }

            for (auto pair: next_level[i]->ranks_)
                (*next_level_ranks[pair.first])[i] = pair.second;

            for (auto pair: next_level[i]->prefix_ranks_)
                (*next_level_prefix_ranks[pair.first])[i] = pair.second;

            for (auto pair: next_level[i]->cumulated_ranks_)
                (*next_level_cumulated_ranks[pair.first])[i] = pair.second;
        }




        sdsl::int_vector<>* current_level_offsets = new sdsl::int_vector<>(number_of_leaves);

        std::unordered_map<int,sdsl::int_vector<>*> current_level_first_ranks;
        std::unordered_map<int,sdsl::int_vector<>*> current_level_second_ranks;
        for (auto pair: current_level[0]->ranks_) {
            current_level_first_ranks[pair.first] = new sdsl::int_vector<>(number_of_leaves);
            current_level_second_ranks[pair.first] = new sdsl::int_vector<>(number_of_leaves);
        }

        int j = 0;
        for (int i = 0; i < current_level.size(); ++i) {
            if (!(*current_level_bv)[i]) {
                for (auto pair: current_level[i]->first_ranks_)
                    (*current_level_first_ranks[pair.first])[j] = pair.second;

                for (auto pair: current_level[i]->second_ranks_)
                    (*current_level_second_ranks[pair.first])[j] = pair.second;

                (*current_level_offsets)[j++] = current_level[i]->first_block_->level_index_ * current_length + current_level[i]->offset_;
            }
        }

        sdsl::util::bit_compress(*current_level_offsets);
        bt_offsets_.push_back(current_level_offsets);
        c_bt_offsets_.push_back(new sdsl::dac_vector_dp<sdsl::rrr_vector<>>(*current_level_offsets));

        for (auto pair : next_level_ranks) {
            sdsl::util::bit_compress(*(pair.second));
            (bt_ranks_[pair.first]).push_back(pair.second);
            (c_bt_ranks_[pair.first]).push_back(new sdsl::dac_vector_dp<sdsl::rrr_vector<>>((*pair.second)));
        }
        for (auto pair : next_level_prefix_ranks) {
            sdsl::util::bit_compress(*(pair.second));
            (bt_prefix_ranks_[pair.first]).push_back(pair.second);
            (c_bt_prefix_ranks_[pair.first]).push_back(new sdsl::dac_vector_dp<sdsl::rrr_vector<>>((*pair.second)));
        }
        for (auto pair : next_level_better_prefix_ranks) {
            sdsl::util::bit_compress(*(pair.second));
            (bt_better_prefix_ranks_[pair.first]).push_back(pair.second);
            (c_bt_better_prefix_ranks_[pair.first]).push_back(new sdsl::dac_vector_dp<sdsl::rrr_vector<>>((*pair.second)));
        }
        for (auto pair : next_level_cumulated_ranks) {
            sdsl::util::bit_compress(*(pair.second));
            (bt_cumulated_ranks_[pair.first]).push_back(pair.second);
            (c_bt_cumulated_ranks_[pair.first]).push_back(new sdsl::dac_vector_dp<sdsl::rrr_vector<>>((*pair.second)));
        }

        for (auto pair : current_level_first_ranks) {
            sdsl::util::bit_compress(*(pair.second));
            (bt_first_ranks_[pair.first]).push_back(pair.second);
            (c_bt_first_ranks_[pair.first]).push_back(new sdsl::dac_vector_dp<sdsl::rrr_vector<>>((*pair.second)));
        }
        for (auto pair : current_level_second_ranks) {
            sdsl::util::bit_compress(*(pair.second));
            (bt_second_ranks_[pair.first]).push_back(pair.second);
            (c_bt_second_ranks_[pair.first]).push_back(new sdsl::dac_vector_dp<sdsl::rrr_vector<>>((*pair.second)));
        }




        bt_bv_.push_back(current_level_bv);
        sdsl::rank_support_v<1>* current_level_bv_rank = new sdsl::rank_support_v<1>(current_level_bv);
        bt_bv_rank_.push_back(current_level_bv_rank);

        c_bt_bv_.push_back(new sdsl::rrr_vector<63>(*bt_bv_.back()));
        c_bt_bv_rank_.push_back(new sdsl::rrr_vector<63>::rank_1_type(c_bt_bv_.back()));

        current_level = next_level;
        next_level = bt->next_level(current_level);
        ++number_of_levels_;
    }

    ++number_of_levels_;

    std::vector<PBlock*> last_level = current_level;

    std::string leaf_string = "";
    for (PBlock* b: last_level) {
        leaf_string += b->represented_string();
    }

    std::unordered_set<char> alphabet;
    for (char c : leaf_string) {
        alphabet.insert(c);
    }
    alphabet_ = new sdsl::int_vector<>(alphabet.size());
    int counter = 0;
    for (char c : alphabet){
        mapping_[c] = counter;
        (*alphabet_)[counter++] = c;
    }
    sdsl::util::bit_compress(*alphabet_);


    leaf_string_ = new sdsl::int_vector<>(leaf_string.size());

    for (int i = 0; i<leaf_string_->size(); ++i) {
        (*leaf_string_)[i] = mapping_[leaf_string[i]];
    }

    leaf_bv_ = new sdsl::bit_vector(leaf_string_->size());
    for (int i = 0; i<leaf_string_->size(); ++i) {
        if (mapping_[leaf_string[i]]) (*leaf_bv_)[i] = 1;
        else (*leaf_bv_)[i] = 0;
    }
    leaf_bv_rank_ = new sdsl::rank_support_v<1>(leaf_bv_);
    c_leaf_bv_ = new sdsl::rrr_vector<63>(*leaf_bv_);
    c_leaf_bv_rank_ = new sdsl::rrr_vector<63>::rank_1_type(c_leaf_bv_);


    sdsl::util::bit_compress(*leaf_string_);
    c_leaf_string_ = new sdsl::dac_vector_dp<sdsl::rrr_vector<>>(*leaf_string_);

    /*
    leaf_string_wt_ = new sdsl::wt_blcd<>();
    construct_im(*leaf_string_wt_, leaf_string, 1);
     */
}

PCBlockTree::~PCBlockTree() {

    for (sdsl::bit_vector* bv : bt_bv_)
        delete bv;
    for (sdsl::rank_support_v<1>* rank : bt_bv_rank_)
        delete rank;


    for (sdsl::int_vector<>* offsets : bt_offsets_)
        delete offsets;

    delete leaf_string_;
    delete alphabet_;
    delete leaf_bv_;
    delete leaf_bv_rank_;
    delete c_leaf_bv_;
    delete c_leaf_bv_rank_;

    for (auto pair : bt_first_level_prefix_ranks_) {
        delete pair.second;
    }

    for (auto pair : bt_first_level_cumulated_ranks_) {
        delete pair.second;
    }

    for (auto pair : bt_prefix_ranks_) {
        for (sdsl::int_vector<>* ranks : pair.second) {
            delete ranks;
        }
    }


    for (auto pair : bt_better_prefix_ranks_) {
        for (sdsl::int_vector<> *ranks : pair.second) {
            delete ranks;
        }
    }

        for (auto pair : bt_cumulated_ranks_) {
        for (sdsl::int_vector<>* ranks : pair.second) {
            delete ranks;
        }
    }

    for (auto pair : bt_ranks_) {
        for (sdsl::int_vector<>* ranks : pair.second) {
            delete ranks;
        }
    }

    for (auto pair : bt_first_ranks_) {
        for (sdsl::int_vector<>* ranks : pair.second) {
            delete ranks;
        }
    }

    for (auto pair : bt_second_ranks_) {
        for (sdsl::int_vector<> *ranks : pair.second) {
            delete ranks;
        }
    }


    // Compressed
    for (sdsl::rrr_vector<63>* bv : c_bt_bv_)
        delete bv;
    for (sdsl::rrr_vector<63>::rank_1_type* rank : c_bt_bv_rank_)
        delete rank;

    for (sdsl::dac_vector_dp<sdsl::rrr_vector<>>* offsets : c_bt_offsets_)
        delete offsets;

    delete c_leaf_string_;

    for (auto pair : c_bt_first_level_prefix_ranks_) {
        delete pair.second;
    }

    for (auto pair : c_bt_first_level_cumulated_ranks_) {
        delete pair.second;
    }

    for (auto pair : c_bt_prefix_ranks_) {
        for (sdsl::dac_vector_dp<sdsl::rrr_vector<>>* ranks : pair.second) {
            delete ranks;
        }
    }


    for (auto pair : c_bt_better_prefix_ranks_) {
        for (sdsl::dac_vector_dp<sdsl::rrr_vector<>>* ranks : pair.second) {
            delete ranks;
        }
    }

    for (auto pair : c_bt_cumulated_ranks_) {
        for (sdsl::dac_vector_dp<sdsl::rrr_vector<>>* ranks : pair.second) {
            delete ranks;
        }
    }

    for (auto pair : c_bt_ranks_) {
        for (sdsl::dac_vector_dp<sdsl::rrr_vector<>>* ranks : pair.second) {
            delete ranks;
        }
    }

    for (auto pair : c_bt_first_ranks_) {
        for (sdsl::dac_vector_dp<sdsl::rrr_vector<>>* ranks : pair.second) {
            delete ranks;
        }
    }

    for (auto pair : c_bt_second_ranks_) {
        for (sdsl::dac_vector_dp<sdsl::rrr_vector<>> *ranks : pair.second) {
            delete ranks;
        }
    }


//    delete leaf_string_wt_;
}

int PCBlockTree::access(int i) {

    int current_block = i/first_level_length_;
    int current_length = first_level_length_;
    i -= current_block*first_level_length_;
    int level = 0;
    while (level < number_of_levels_-1) {
        if ((*bt_bv_[level])[current_block]) { // Case InternalBlock
            current_length /= r_;
            int child_number = i/current_length;
            i -= child_number*current_length;
            current_block = (*bt_bv_rank_[level])(current_block)*r_ + child_number;
            ++level;
        } else { // Case BackBlock
            int encoded_offset = (*bt_offsets_[level])[current_block-(*bt_bv_rank_[level])(current_block+1)];
            current_block = encoded_offset/current_length;
            i += encoded_offset%current_length;
            if (i >= current_length) {
                ++current_block;
                i -= current_length;
            }
        }
    }
    return (*alphabet_)[(*leaf_string_)[i+current_block*current_length]];
}


int PCBlockTree::c_access(int i) {

    int current_block = i/first_level_length_;
    int current_length = first_level_length_;
    i -= current_block*first_level_length_;
    int level = 0;
    while (level < number_of_levels_-1) {
        if ((*c_bt_bv_[level])[current_block]) { // Case InternalBlock
            current_length /= r_;
            int child_number = i/current_length;
            i -= child_number*current_length;
            current_block = (*c_bt_bv_rank_[level])(current_block)*r_ + child_number;
            ++level;
        } else { // Case BackBlock
            int encoded_offset = (*c_bt_offsets_[level])[current_block-(*c_bt_bv_rank_[level])(current_block+1)];
            current_block = encoded_offset/current_length;
            i += encoded_offset%current_length;
            if (i >= current_length) {
                ++current_block;
                i -= current_length;
            }
        }
    }
    return (*alphabet_)[(*c_leaf_string_)[i+current_block*current_length]];
}






int PCBlockTree::rank(int c, int i) {
    auto& prefix_ranks = bt_prefix_ranks_[c];
    auto& second_ranks = bt_second_ranks_[c];
    auto& first_ranks = bt_first_ranks_[c];

    int current_block = i/first_level_length_;
    int current_length = first_level_length_;
    i -= current_block*current_length;
    int level = 0;
    int r = (*bt_first_level_prefix_ranks_[c])[current_block];

    while (level < number_of_levels_-1) {
        if ((*bt_bv_[level])[current_block]) { // Case InternalBlock
            current_length /= r_;
            int child_number = i/current_length;
            i -= child_number*current_length;
            current_block = (*bt_bv_rank_[level])(current_block)*r_ + child_number;
            ++level;
            r += (*prefix_ranks[level-1])[current_block];
        } else { // Case BackBlock
            int index = current_block-(*bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*bt_offsets_[level])[index];
            current_block = encoded_offset/current_length;
            i += encoded_offset%current_length;
            if (i >= current_length) {
                r += (*second_ranks[level])[index];
                ++current_block;
                i -= current_length;
            } else {
                r -= (*first_ranks[level])[index];
            }
        }
    }

    i  += current_block*current_length;
    int d = mapping_[c];
    for (int j = current_block*current_length; j <= i; ++j) {
        if ((*leaf_string_)[j] == d) ++r;
    }

    return r;
     /*
    int qq = (onesRank*max_length_level + zerosRank*(max_length_level-1));
    int rr = (onesRank*max_length_level + zerosRank*(max_length_level-1)) + i;

    return r + leaf_string_wt_->rank(rr+1, c) - leaf_string_wt_->rank(qq, c);
     */
}


int PCBlockTree::better_paper_rank(int c, int i) {
    auto &prefix_ranks = bt_better_prefix_ranks_[c];
    auto &second_ranks = bt_second_ranks_[c];
    auto& first_cumulated_ranks = bt_first_level_cumulated_ranks_[c];
    //auto& first_ranks = bt_first_ranks_[c];

    int current_block = i/first_level_length_;
    int current_length = first_level_length_;
    i -= current_block*current_length;
    int level = 0;
    int r = (current_block == 0) ? 0 : (*first_cumulated_ranks)[current_block-1];

    while (level < number_of_levels_-1) {
        if ((*bt_bv_[level])[current_block]) { // Case InternalBlock
            current_length /= r_;
            int child_number = i/current_length;
            i -= child_number*current_length;
            current_block = (*bt_bv_rank_[level])(current_block)*r_ + child_number;
            ++level;
            r += (current_block%r_ == 0) ? 0 : (*prefix_ranks[level-1])[current_block];
        } else { // Case BackBlock
            int index = current_block-(*bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*bt_offsets_[level])[index];
            current_block = encoded_offset/current_length;
            i += encoded_offset%current_length;
            r += (*second_ranks[level])[index];
            if (i >= current_length) {
                ++current_block;
                i -= current_length;
            } else {
                if (level != 0 ) {
                    r -= ((*prefix_ranks[level - 1])[(current_block + 1) % (prefix_ranks[level - 1]->size())]);
                    if (current_block%r_ != 0 && current_block+1 != prefix_ranks[level - 1]->size() && (current_block+1)%r_ != 0) {
                        r += ((*prefix_ranks[level - 1])[current_block]);
                    }
                }
                else r -= ((current_block == 0) ?  ((*first_cumulated_ranks)[current_block]) :  (((*first_cumulated_ranks)[current_block]) - ((*first_cumulated_ranks)[current_block-1])) );
                //r -= (*first_ranks[level])[index];
            }
        }
    }

    i  += current_block*current_length;
    int d = mapping_[c];
    for (int j = current_block*current_length; j <= i; ++j) {
        if ((*leaf_string_)[j] == d) ++r;
    }

    return r;
    /*
   int qq = (onesRank*max_length_level + zerosRank*(max_length_level-1));
   int rr = (onesRank*max_length_level + zerosRank*(max_length_level-1)) + i;

   return r + leaf_string_wt_->rank(rr+1, c) - leaf_string_wt_->rank(qq, c);
    */
}





int PCBlockTree::c_better_paper_rank(int c, int i) {
    auto &prefix_ranks = c_bt_better_prefix_ranks_[c];
    auto &second_ranks = c_bt_second_ranks_[c];
    auto& first_cumulated_ranks = c_bt_first_level_cumulated_ranks_[c];
    //auto& first_ranks = bt_first_ranks_[c];

    int current_block = i/first_level_length_;
    int current_length = first_level_length_;
    i -= current_block*current_length;
    int level = 0;
    int r = (current_block == 0) ? 0 : (*first_cumulated_ranks)[current_block-1];

    while (level < number_of_levels_-1) {
        if ((*c_bt_bv_[level])[current_block]) { // Case InternalBlock
            current_length /= r_;
            int child_number = i/current_length;
            i -= child_number*current_length;
            current_block = (*c_bt_bv_rank_[level])(current_block)*r_ + child_number;
            ++level;
            r += (current_block%r_ == 0) ? 0 : (*prefix_ranks[level-1])[current_block];
        } else { // Case BackBlock
            int index = current_block-(*c_bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*c_bt_offsets_[level])[index];
            current_block = encoded_offset/current_length;
            i += encoded_offset%current_length;
            r += (*second_ranks[level])[index];
            if (i >= current_length) {
                ++current_block;
                i -= current_length;
            } else {
                if (level != 0 ) {
                    r -= ((*prefix_ranks[level - 1])[(current_block + 1) % (prefix_ranks[level - 1]->size())]);
                    if (current_block%r_ != 0 && current_block+1 != prefix_ranks[level - 1]->size() && (current_block+1)%r_ != 0) {
                        r += ((*prefix_ranks[level - 1])[current_block]);
                    }
                }
                else r -= ((current_block == 0) ?  ((*first_cumulated_ranks)[current_block]) :  (((*first_cumulated_ranks)[current_block]) - ((*first_cumulated_ranks)[current_block-1])) );
                //r -= (*first_ranks[level])[index];
            }
        }
    }

    i  += current_block*current_length;
    int d = mapping_[c];
    for (int j = current_block*current_length; j <= i; ++j) {
        if ((*c_leaf_string_)[j] == d) ++r;
    }

    return r;
    /*
   int qq = (onesRank*max_length_level + zerosRank*(max_length_level-1));
   int rr = (onesRank*max_length_level + zerosRank*(max_length_level-1)) + i;

   return r + leaf_string_wt_->rank(rr+1, c) - leaf_string_wt_->rank(qq, c);
    */
}

int PCBlockTree::select(int c, int k) {

    auto& prefix_ranks = bt_prefix_ranks_[c];
     auto& second_ranks = bt_second_ranks_[c];
    auto& first_ranks = bt_first_ranks_[c];
    auto& first_level_prefix_ranks = bt_first_level_prefix_ranks_[c];

    int current_block = (k-1)/first_level_length_;

    int end_block = number_of_blocks_first_level_-1;
    while (current_block != end_block) {
        int m = current_block + (end_block-current_block)/2;
        int f = (*first_level_prefix_ranks)[m];
        if (f < k) {
            if (end_block - current_block == 1) {
                if ((*first_level_prefix_ranks)[m+1] < k) {
                    current_block = m+1;
                }
                break;
            }
            current_block = m;
        } else {
            end_block = m-1;
        }
    }
    /* // Change for first_level_prefix_ranks
    while (initial_block < bt_bv_[0]->size()-1 && k > (*bt_prefix_ranks_[c][0])[initial_block+1]) {
        ++initial_block;
    }
     */

    int s = current_block*first_level_length_;
    k -= (*first_level_prefix_ranks)[current_block];
    int current_length = first_level_length_;
    int level = 0;
    while (level < number_of_levels_-1) {
        if ((*bt_bv_[level])[current_block]) { // Case InternalBlock
            int firstChild = (*bt_bv_rank_[level])(current_block)*r_;
            int child = firstChild;
            /* // Fix this to counter - 1, because of firstlevelprefixranks_ , and factorized map
            int first = firstChild;
            int last = firstChild + r_ - 1;

            while (true) {
                int m = first + (last-first)/2;
                if (m == firstChild+r_ - 1) {
                    child = m;
                    break;
                }
                int f = (*bt_prefix_ranks_[c][counter+1])[m];
                int s = (*bt_prefix_ranks_[c][counter+1])[m+1];
                if (f<k && s >= k) {
                    child = m;
                    break;
                }

                if (f >= k) {
                    last = m-1;
                } else {
                    first = m+1;
                }
            }
            */
            /*
            int end_child = firstChild + r_-1;
            while (child != end_child) {
                int m = child + (end_child-child)/2;
                int f = (*prefix_ranks[counter+1-1])[m];
                if (f < k) {
                    if (end_child - child == 1) {
                        if ((*prefix_ranks[counter+1-1])[m+1] < k) {
                            child = m+1;
                        }
                        break;
                    }
                    child = m;
                } else {
                    end_child = m-1;
                }
            }
            */
             // Sequential search behaves better
            int last_possible_child = (firstChild + r_-1 > (*prefix_ranks[level+1-1]).size()-1) ?  (*prefix_ranks[level+1-1]).size()-1 : firstChild + r_-1;
            while ( child <  last_possible_child && k > (*prefix_ranks[level+1-1])[child+1]) {
                ++child;
            }
            k -= (*prefix_ranks[level+1-1])[child];
            current_length /= r_;
            s += (child-firstChild)*current_length;
            current_block = child;
            ++level;
        } else { // Case BackBlock
            int index = current_block - (*bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*bt_offsets_[level])[index];
            current_block = encoded_offset/current_length;
            s -= encoded_offset%current_length;
            int second_rank = (*second_ranks[level])[index];
            if (k > second_rank) {
                s += current_length;
                k-= second_rank;
                ++current_block;
            } else {
                k += (*first_ranks[level])[index];
            }
        }
    }


    int d = mapping_[c];
    for (int j = current_block*current_length;  ; ++j) {
        if ((*leaf_string_)[j] == d) --k;
        if (!k) return s + j - current_block*current_length;
    }
/*
    int qq = (onesRank*max_length_level + zerosRank*(max_length_level-1));
    return s+leaf_string_wt_->select(k + leaf_string_wt_->rank(qq, c), c)-qq;
*/
    return -1;
}



int PCBlockTree::better_paper_select(int c, int k) {

    auto& prefix_ranks = bt_better_prefix_ranks_[c];
    auto& second_ranks = bt_second_ranks_[c];
    //auto& first_ranks = bt_first_ranks_[c];
    auto& first_level_cumulated_ranks = bt_first_level_cumulated_ranks_[c];

    int current_block = (k-1)/first_level_length_;

    int end_block = number_of_blocks_first_level_-1;
    while (current_block != end_block) {
        int m = current_block + (end_block-current_block)/2;
        int f = (*first_level_cumulated_ranks)[m];
        if (f < k) {
            /*
            if (end_block - initial_block == 1) {
                if ((*first_level_prefix_ranks)[m+1] < k) {
                    initial_block = m+1;
                }
                break;
            }
            initial_block = m;
             */
            current_block = m+1;
        } else {
            end_block = m;

        }
    }
    /* // Change for first_level_prefix_ranks
    while (initial_block < bt_bv_[0]->size()-1 && k > (*bt_prefix_ranks_[c][0])[initial_block+1]) {
        ++initial_block;
    }
     */

    int s = current_block*first_level_length_;
    k -= (current_block == 0) ? 0 : (*first_level_cumulated_ranks)[current_block-1];


    int current_length = first_level_length_;
    int level = 0;
    while (level < number_of_levels_-1) {
        if ((*bt_bv_[level])[current_block]) { // Case InternalBlock
            int firstChild = (*bt_bv_rank_[level])(current_block)*r_;
            int child = firstChild;
            /* // Fix this to counter - 1, because of firstlevelprefixranks_ , and factorized map
            int first = firstChild;
            int last = firstChild + r_ - 1;

            while (true) {
                int m = first + (last-first)/2;
                if (m == firstChild+r_ - 1) {
                    child = m;
                    break;
                }
                int f = (*bt_prefix_ranks_[c][counter+1])[m];
                int s = (*bt_prefix_ranks_[c][counter+1])[m+1];
                if (f<k && s >= k) {
                    child = m;
                    break;
                }

                if (f >= k) {
                    last = m-1;
                } else {
                    first = m+1;
                }
            }
            */
            /*
            int end_child = firstChild + r_-1;
            while (child != end_child) {
                int m = child + (end_child-child)/2;
                int f = (*prefix_ranks[counter+1-1])[m];
                if (f < k) {
                    if (end_child - child == 1) {
                        if ((*prefix_ranks[counter+1-1])[m+1] < k) {
                            child = m+1;
                        }
                        break;
                    }
                    child = m;
                } else {
                    end_child = m-1;
                }
            }
            */
            // Sequential search behaves better
            int last_possible_child = (firstChild + r_-1 > (*prefix_ranks[level+1-1]).size()-1) ?  (*prefix_ranks[level+1-1]).size()-1 : firstChild + r_-1;
            while ( child <  last_possible_child && k > (((child+1)%r_ == 0) ? 0 : (*prefix_ranks[level+1-1])[child+1])/*(*prefix_ranks[level+1-1])[child+1]*/) {
                ++child;
            }
            k -= (child%r_ == 0) ? 0 : (*prefix_ranks[level+1-1])[child];
            current_length /= r_;
            s += (child-firstChild)*current_length;
            current_block = child;
            ++level;
        } else { // Case BackBlock
            int index = current_block - (*bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*bt_offsets_[level])[index];
            current_block = encoded_offset/current_length;
            s -= encoded_offset%current_length;
            k-= (*second_ranks[level])[index];
            if (k > 0) {
                s += current_length;
                ++current_block;
            } else {
                if (level != 0 ) {
                    k += ((*prefix_ranks[level - 1])[(current_block + 1) % (prefix_ranks[level - 1]->size())]);
                    if (current_block%r_ != 0 && current_block+1 != prefix_ranks[level - 1]->size() && (current_block+1)%r_ != 0) {
                        k -= ((*prefix_ranks[level - 1])[current_block]);
                    }
                }
                else k += ((current_block == 0) ?  ((*first_level_cumulated_ranks)[current_block]) :  (((*first_level_cumulated_ranks)[current_block]) - ((*first_level_cumulated_ranks)[current_block-1])) );
            }
        }
    }


    int d = mapping_[c];
    for (int j = current_block*current_length;  ; ++j) {
        if ((*leaf_string_)[j] == d) --k;
        if (!k) return s + j - current_block*current_length;
    }
/*
    int qq = (onesRank*max_length_level + zerosRank*(max_length_level-1));
    return s+leaf_string_wt_->select(k + leaf_string_wt_->rank(qq, c), c)-qq;
*/
    return -1;
}



int PCBlockTree::c_better_paper_select(int c, int k) {

    auto& prefix_ranks = c_bt_better_prefix_ranks_[c];
    auto& second_ranks = c_bt_second_ranks_[c];
    //auto& first_ranks = bt_first_ranks_[c];
    auto& first_level_cumulated_ranks = c_bt_first_level_cumulated_ranks_[c];

    int current_block = (k-1)/first_level_length_;

    int end_block = number_of_blocks_first_level_-1;
    while (current_block != end_block) {
        int m = current_block + (end_block-current_block)/2;
        int f = (*first_level_cumulated_ranks)[m];
        if (f < k) {
            /*
            if (end_block - initial_block == 1) {
                if ((*first_level_prefix_ranks)[m+1] < k) {
                    initial_block = m+1;
                }
                break;
            }
            initial_block = m;
             */
            current_block = m+1;
        } else {
            end_block = m;

        }
    }
    /* // Change for first_level_prefix_ranks
    while (initial_block < bt_bv_[0]->size()-1 && k > (*bt_prefix_ranks_[c][0])[initial_block+1]) {
        ++initial_block;
    }
     */

    int s = current_block*first_level_length_;
    k -= (current_block == 0) ? 0 : (*first_level_cumulated_ranks)[current_block-1];


    int current_length = first_level_length_;
    int level = 0;
    while (level < number_of_levels_-1) {
        if ((*c_bt_bv_[level])[current_block]) { // Case InternalBlock
            int firstChild = (*c_bt_bv_rank_[level])(current_block)*r_;
            int child = firstChild;
            /* // Fix this to counter - 1, because of firstlevelprefixranks_ , and factorized map
            int first = firstChild;
            int last = firstChild + r_ - 1;

            while (true) {
                int m = first + (last-first)/2;
                if (m == firstChild+r_ - 1) {
                    child = m;
                    break;
                }
                int f = (*bt_prefix_ranks_[c][counter+1])[m];
                int s = (*bt_prefix_ranks_[c][counter+1])[m+1];
                if (f<k && s >= k) {
                    child = m;
                    break;
                }

                if (f >= k) {
                    last = m-1;
                } else {
                    first = m+1;
                }
            }
            */
            /*
            int end_child = firstChild + r_-1;
            while (child != end_child) {
                int m = child + (end_child-child)/2;
                int f = (*prefix_ranks[counter+1-1])[m];
                if (f < k) {
                    if (end_child - child == 1) {
                        if ((*prefix_ranks[counter+1-1])[m+1] < k) {
                            child = m+1;
                        }
                        break;
                    }
                    child = m;
                } else {
                    end_child = m-1;
                }
            }
            */
            // Sequential search behaves better
            int last_possible_child = (firstChild + r_-1 > (*prefix_ranks[level+1-1]).size()-1) ?  (*prefix_ranks[level+1-1]).size()-1 : firstChild + r_-1;
            while ( child <  last_possible_child && k > (((child+1)%r_ == 0) ? 0 : (*prefix_ranks[level+1-1])[child+1])/*(*prefix_ranks[level+1-1])[child+1]*/) {
                ++child;
            }
            k -= (child%r_ == 0) ? 0 : (*prefix_ranks[level+1-1])[child];
            current_length /= r_;
            s += (child-firstChild)*current_length;
            current_block = child;
            ++level;
        } else { // Case BackBlock
            int index = current_block - (*c_bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*c_bt_offsets_[level])[index];
            current_block = encoded_offset/current_length;
            s -= encoded_offset%current_length;
            k-= (*second_ranks[level])[index];
            if (k > 0) {
                s += current_length;
                ++current_block;
            } else {
                if (level != 0 ) {
                    k += ((*prefix_ranks[level - 1])[(current_block + 1) % (prefix_ranks[level - 1]->size())]);
                    if (current_block%r_ != 0 && current_block+1 != prefix_ranks[level - 1]->size() && (current_block+1)%r_ != 0) {
                        k -= ((*prefix_ranks[level - 1])[current_block]);
                    }
                }
                else k += ((current_block == 0) ?  ((*first_level_cumulated_ranks)[current_block]) :  (((*first_level_cumulated_ranks)[current_block]) - ((*first_level_cumulated_ranks)[current_block-1])) );
            }
        }
    }


    int d = mapping_[c];
    for (int j = current_block*current_length;  ; ++j) {
        if ((*c_leaf_string_)[j] == d) --k;
        if (!k) return s + j - current_block*current_length;
    }
/*
    int qq = (onesRank*max_length_level + zerosRank*(max_length_level-1));
    return s+leaf_string_wt_->select(k + leaf_string_wt_->rank(qq, c), c)-qq;
*/
    return -1;
}

int PCBlockTree::c_noback_rank_alternative(int c, int i) {

    int current_block = i/first_level_length_;
    return (*c_bt_first_level_prefix_ranks_[c])[current_block] + c_recursive_noback_rank_alternative(c, i-current_block*first_level_length_, 0, current_block, first_level_length_);
}


int PCBlockTree::c_recursive_noback_rank_alternative(int c, int i, int level, int level_index, int level_length) {
    if (i == -1) return 0;
    int r = 0;
    if (level == number_of_levels_-1) {

        i  += level_index*level_length;
        int d = mapping_[c];
        for (int j = level_index*level_length; j <= i; ++j) {
            if ((*c_leaf_string_)[j] == d) ++r;
        }

        return r;
    }
    auto& ranks = c_bt_ranks_[c];


    if ((*c_bt_bv_[level])[level_index]) { // Case InternalBlock
        level_length /= r_;
        int child_number = i/level_length;
        i -= child_number*level_length;

        int firstChild = (*c_bt_bv_rank_[level])(level_index)*r_;
        for (int child = firstChild; child < firstChild + child_number; ++child)
            r += (*ranks[level+1])[child];
        level_index = firstChild + child_number;
        ++level;
    } else { // Case BackBlock
        int index = level_index - (*c_bt_bv_rank_[level])(level_index+1);
        int encoded_offset = (*c_bt_offsets_[level])[index];
        level_index = encoded_offset/level_length;
        i += encoded_offset%level_length;
        r -= c_recursive_noback_rank_alternative(c, encoded_offset%level_length - 1, level, level_index, level_length);
        if (i >= level_length) {
            r += (*ranks[level])[level_index];
            ++level_index;
            i -= level_length;
        }
    }

    return r + c_recursive_noback_rank_alternative(c, i, level, level_index, level_length);
}

int PCBlockTree::noback_rank_alternative(int c, int i) {

    int current_block = i/first_level_length_;
    return (*bt_first_level_prefix_ranks_[c])[current_block] + recursive_noback_rank_alternative(c, i-current_block*first_level_length_, 0, current_block, first_level_length_);

}

int PCBlockTree::recursive_noback_rank_alternative(int c, int i, int level, int level_index, int level_length) {
    if (i == -1) return 0;
    int r = 0;
    if (level == number_of_levels_-1) {

        i  += level_index*level_length;
        int d = mapping_[c];
        for (int j = level_index*level_length; j <= i; ++j) {
            if ((*leaf_string_)[j] == d) ++r;
        }

        return r;
    }
    auto& ranks = bt_ranks_[c];


    if ((*bt_bv_[level])[level_index]) { // Case InternalBlock
        level_length /= r_;
        int child_number = i/level_length;
        i -= child_number*level_length;

        int firstChild = (*bt_bv_rank_[level])(level_index)*r_;
        for (int child = firstChild; child < firstChild + child_number; ++child)
            r += (*ranks[level+1])[child];
        level_index = firstChild + child_number;
        ++level;
    } else { // Case BackBlock
        int index = level_index - (*bt_bv_rank_[level])(level_index+1);
        int encoded_offset = (*bt_offsets_[level])[index];
        level_index = encoded_offset/level_length;
        i += encoded_offset%level_length;
        r -= recursive_noback_rank_alternative(c, encoded_offset%level_length - 1, level, level_index, level_length);
        if (i >= level_length) {
            r += (*ranks[level])[level_index];
            ++level_index;
            i -= level_length;
        }
    }

    return r + recursive_noback_rank_alternative(c, i, level, level_index, level_length);
}


int PCBlockTree::rank_alternative(int c, int i) {

    auto& ranks = bt_ranks_[c];
    auto& second_ranks = bt_second_ranks_[c];

    int current_block = i/first_level_length_;
    int current_length = first_level_length_;
    i = i-current_block*current_length;
    int level = 0;

    int r = (*bt_first_level_prefix_ranks_[c])[current_block];
    while (level < number_of_levels_-1) {
        if ((*bt_bv_[level])[current_block]) { // Case InternalBlock
            current_length /= r_;
            int child_number = i/current_length;
            i -= child_number*current_length;

            int firstChild = (*bt_bv_rank_[level])(current_block)*r_;
            for (int child = firstChild; child < firstChild + child_number; ++child)
                r += (*ranks[level+1])[child];
            current_block = firstChild + child_number;
            ++level;
        } else { // Case BackBlock
            int index = current_block - (*bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*bt_offsets_[level])[index];
            current_block = encoded_offset/current_length;
            i += encoded_offset%current_length;
            r += (*second_ranks[level])[index];
            if (i >= current_length) {
                ++current_block;
                i -= current_length;
            } else {
                r -= (*ranks[level])[current_block];
            }

        }
    }

    i  += current_block*current_length;
    int d = mapping_[c];
    for (int j = current_block*current_length; j <= i; ++j) {
        if ((*leaf_string_)[j] == d) ++r;
    }

    return r;
}



int PCBlockTree::optimized_rank_alternative(int c, int i) {

    auto& ranks = bt_ranks_[c];
    auto& second_ranks = bt_second_ranks_[c];

    int current_block = i/first_level_length_;
    int current_length = first_level_length_;
    i = i-current_block*current_length;
    int level = 0;

    int r = (*bt_first_level_prefix_ranks_[c])[current_block];
    while (level < number_of_levels_-1) {
        if ((i+1)% current_length == 0) {// or it is the last in the last block
            return r + (*ranks[level])[current_block];
        }
        if ((*bt_bv_[level])[current_block]) { // Case InternalBlock
            current_length /= r_;
            int child_number = i/current_length;
            i -= child_number*current_length;

            int firstChild = (*bt_bv_rank_[level])(current_block)*r_;
            for (int child = firstChild; child < firstChild + child_number; ++child)
                r += (*ranks[level+1])[child];
            current_block = firstChild + child_number;
            ++level;
        } else { // Case BackBlock
            int index = current_block - (*bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*bt_offsets_[level])[index];
            current_block = encoded_offset/current_length;
            i += encoded_offset%current_length;
            r += (*second_ranks[level])[index];
            if (i >= current_length) {
                ++current_block;
                i -= current_length;
            } else {
                r -= (*ranks[level])[current_block];
            }

        }
    }

    i  += current_block*current_length;
    int d = mapping_[c];
    for (int j = current_block*current_length; j <= i; ++j) {
        if ((*leaf_string_)[j] == d) ++r;
    }

    return r;
}

int PCBlockTree::c_rank_alternative(int c, int i) {

    auto& ranks = c_bt_ranks_[c];
    auto& second_ranks = c_bt_second_ranks_[c];

    int current_block = i/first_level_length_;
    int current_length = first_level_length_;
    i = i-current_block*current_length;
    int level = 0;

    int r = (*bt_first_level_prefix_ranks_[c])[current_block];
    while (level < number_of_levels_-1) {
        if ((*c_bt_bv_[level])[current_block]) { // Case InternalBlock
            current_length /= r_;
            int child_number = i/current_length;
            i -= child_number*current_length;

            int firstChild = (*c_bt_bv_rank_[level])(current_block)*r_;
            for (int child = firstChild; child < firstChild + child_number; ++child)
                r += (*ranks[level+1])[child];
            current_block = firstChild + child_number;
            ++level;
        } else { // Case BackBlock
            int index = current_block - (*c_bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*c_bt_offsets_[level])[index];
            current_block = encoded_offset/current_length;
            i += encoded_offset%current_length;
            r += (*second_ranks[level])[index];
            if (i >= current_length) {
                ++current_block;
                i -= current_length;
            } else {
                r -= (*ranks[level])[current_block];
            }

        }
    }

    i  += current_block*current_length;
    int d = mapping_[c];
    for (int j = current_block*current_length; j <= i; ++j) {
        if ((*c_leaf_string_)[j] == d) ++r;
    }

    return r;
}

int PCBlockTree::noback_select_alternative(int c, int k) {
    auto& ranks = bt_ranks_[c];
    auto& first_level_prefix_ranks = bt_first_level_prefix_ranks_[c];


    int current_block = (k-1)/first_level_length_;

    int end_block = number_of_blocks_first_level_-1;
    while (current_block != end_block) {
        int m = current_block + (end_block-current_block)/2;
        int f = (*first_level_prefix_ranks)[m];
        if (f < k) {
            if (end_block - current_block == 1) {
                if ((*first_level_prefix_ranks)[m+1] < k) {
                    current_block = m+1;
                }
                break;
            }
            current_block = m;
        } else {
            end_block = m-1;
        }
    }
    /* // Factorize
    while (initial_block < bt_bv_[0]->size()-1 && k > (*bt_prefix_ranks_[c][0])[initial_block+1]) {
        ++initial_block;
    }
     */
    int current_length = first_level_length_;
    int s = current_block*current_length;
    k -= (*first_level_prefix_ranks)[current_block];
    int level = 0;
    while (level < number_of_levels_-1) {
        if ((*bt_bv_[level])[current_block]) { // Case InternalBlock
            int firstChild = (*bt_bv_rank_[level])(current_block)*r_;
            int child = firstChild;
            int r = (*ranks[level+1])[child];
            int last_possible_child = (firstChild + r_-1 > (*ranks[level+1]).size()-1) ?  (*ranks[level+1]).size()-1 : firstChild + r_-1;
            while ( child < last_possible_child && k > r) { //Border conditions?
                ++child;
                r+= (*ranks[level+1])[child];
            }
            k -= r - (*ranks[level+1])[child];
            current_length /= r_;
            s += (child-firstChild)*current_length;
            current_block = child;
            ++level;
        } else { // Case BackBlock
            int index = current_block -  (*bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*bt_offsets_[level])[index];
            current_block = encoded_offset/current_length;

            int rk = (*ranks[level])[current_block];
            k +=  recursive_noback_rank_alternative(c, encoded_offset%current_length - 1, level, current_block, current_length);
            s -= encoded_offset%current_length;
            if (k > rk) {
                k -= rk;
                s += current_length;
                ++current_block;
            }
        }
    }

    int d = mapping_[c];
    for (int j = current_block*current_length;  ; ++j) {
        if ((*leaf_string_)[j] == d) --k;
        if (!k) return s + j - current_block*current_length;
    }

    return -1;
}

int PCBlockTree::c_noback_select_alternative(int c, int k) {
    auto& ranks = c_bt_ranks_[c];
    auto& first_level_prefix_ranks = c_bt_first_level_prefix_ranks_[c];


    int current_block = (k-1)/first_level_length_;

    int end_block = number_of_blocks_first_level_-1;
    while (current_block != end_block) {
        int m = current_block + (end_block-current_block)/2;
        int f = (*first_level_prefix_ranks)[m];
        if (f < k) {
            if (end_block - current_block == 1) {
                if ((*first_level_prefix_ranks)[m+1] < k) {
                    current_block = m+1;
                }
                break;
            }
            current_block = m;
        } else {
            end_block = m-1;
        }
    }
    /* // Factorize
    while (initial_block < bt_bv_[0]->size()-1 && k > (*bt_prefix_ranks_[c][0])[initial_block+1]) {
        ++initial_block;
    }
     */
    int current_length = first_level_length_;
    int s = current_block*current_length;
    k -= (*first_level_prefix_ranks)[current_block];
    int level = 0;
    while (level < number_of_levels_-1) {
        if ((*c_bt_bv_[level])[current_block]) { // Case InternalBlock
            int firstChild = (*c_bt_bv_rank_[level])(current_block)*r_;
            int child = firstChild;
            int r = (*ranks[level+1])[child];
            int last_possible_child = (firstChild + r_-1 > (*ranks[level+1]).size()-1) ?  (*ranks[level+1]).size()-1 : firstChild + r_-1;
            while ( child < last_possible_child && k > r) { //Border conditions?
                ++child;
                r+= (*ranks[level+1])[child];
            }
            k -= r - (*ranks[level+1])[child];
            current_length /= r_;
            s += (child-firstChild)*current_length;
            current_block = child;
            ++level;
        } else { // Case BackBlock
            int index = current_block -  (*c_bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*c_bt_offsets_[level])[index];
            current_block = encoded_offset/current_length;

            int rk = (*ranks[level])[current_block];
            k +=  c_recursive_noback_rank_alternative(c, encoded_offset%current_length - 1, level, current_block, current_length);
            s -= encoded_offset%current_length;
            if (k > rk) {
                k -= rk;
                s += current_length;
                ++current_block;
            }
        }
    }

    int d = mapping_[c];
    for (int j = current_block*current_length;  ; ++j) {
        if ((*c_leaf_string_)[j] == d) --k;
        if (!k) return s + j - current_block*current_length;
    }

    return -1;
}


int PCBlockTree::select_alternative(int c, int k) {

    auto& ranks = bt_ranks_[c];
    auto& second_ranks = bt_second_ranks_[c];
    auto& first_level_prefix_ranks = bt_first_level_prefix_ranks_[c];


    int current_block = (k-1)/first_level_length_;

    int end_block = number_of_blocks_first_level_-1;
    while (current_block != end_block) {
        int m = current_block + (end_block-current_block)/2;
        int f = (*first_level_prefix_ranks)[m];
        if (f < k) {
            if (end_block - current_block == 1) {
                if ((*first_level_prefix_ranks)[m+1] < k) {
                    current_block = m+1;
                }
                break;
            }
            current_block = m;
        } else {
            end_block = m-1;
        }
    }
    /* // Factorize
    while (initial_block < bt_bv_[0]->size()-1 && k > (*bt_prefix_ranks_[c][0])[initial_block+1]) {
        ++initial_block;
    }
     */
    int current_length = first_level_length_;
    int s = current_block*current_length;
    k -= (*first_level_prefix_ranks)[current_block];
    int level = 0;
    while (level < number_of_levels_-1) {
        if ((*bt_bv_[level])[current_block]) { // Case InternalBlock
            int firstChild = (*bt_bv_rank_[level])(current_block)*r_;
            int child = firstChild;
            int r = (*ranks[level+1])[child];
            int last_possible_child = (firstChild + r_-1 > (*ranks[level+1]).size()-1) ?  (*ranks[level+1]).size()-1 : firstChild + r_-1;
            while ( child < last_possible_child && k > r) { //Border conditions?
                ++child;
                r+= (*ranks[level+1])[child];
            }
            k -= r - (*ranks[level+1])[child];
            current_length /= r_;
            s += (child-firstChild)*current_length;
            current_block = child;
            ++level;
        } else { // Case BackBlock
            int index = current_block -  (*bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*bt_offsets_[level])[index];
            current_block = encoded_offset/current_length;

            k -= (*second_ranks[level])[index];
            s -= encoded_offset%current_length;
            if (k > 0) {
                s += current_length;
                ++current_block;
            } else {
                k += (*ranks[level])[current_block];
            }

        }
    }

    int d = mapping_[c];
    for (int j = current_block*current_length;  ; ++j) {
        if ((*leaf_string_)[j] == d) --k;
        if (!k) return s + j - current_block*current_length;
    }

    return -1;
}


int PCBlockTree::c_select_alternative(int c, int k) {

    auto& ranks = c_bt_ranks_[c];
    auto& second_ranks = c_bt_second_ranks_[c];
    auto& first_level_prefix_ranks = c_bt_first_level_prefix_ranks_[c];


    int current_block = (k-1)/first_level_length_;

    int end_block = number_of_blocks_first_level_-1;
    while (current_block != end_block) {
        int m = current_block + (end_block-current_block)/2;
        int f = (*first_level_prefix_ranks)[m];
        if (f < k) {
            if (end_block - current_block == 1) {
                if ((*first_level_prefix_ranks)[m+1] < k) {
                    current_block = m+1;
                }
                break;
            }
            current_block = m;
        } else {
            end_block = m-1;
        }
    }
    /* // Factorize
    while (initial_block < bt_bv_[0]->size()-1 && k > (*bt_prefix_ranks_[c][0])[initial_block+1]) {
        ++initial_block;
    }
     */
    int current_length = first_level_length_;
    int s = current_block*current_length;
    k -= (*first_level_prefix_ranks)[current_block];
    int level = 0;
    while (level < number_of_levels_-1) {
        if ((*c_bt_bv_[level])[current_block]) { // Case InternalBlock
            int firstChild = (*c_bt_bv_rank_[level])(current_block)*r_;
            int child = firstChild;
            int r = (*ranks[level+1])[child];
            int last_possible_child = (firstChild + r_-1 > (*ranks[level+1]).size()-1) ?  (*ranks[level+1]).size()-1 : firstChild + r_-1;
            while ( child < last_possible_child && k > r) { //Border conditions?
                ++child;
                r+= (*ranks[level+1])[child];
            }
            k -= r - (*ranks[level+1])[child];
            current_length /= r_;
            s += (child-firstChild)*current_length;
            current_block = child;
            ++level;
        } else { // Case BackBlock
            int index = current_block -  (*c_bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*c_bt_offsets_[level])[index];
            current_block = encoded_offset/current_length;

            k -= (*second_ranks[level])[index];
            s -= encoded_offset%current_length;
            if (k > 0) {
                s += current_length;
                ++current_block;
            } else {
                k += (*ranks[level])[current_block];
            }

        }
    }

    int d = mapping_[c];
    for (int j = current_block*current_length;  ; ++j) {
        if ((*c_leaf_string_)[j] == d) --k;
        if (!k) return s + j - current_block*current_length;
    }

    return -1;
}


int PCBlockTree::better_rank(int c, int i) {

    auto& cumulated_ranks = bt_cumulated_ranks_[c];
    auto& second_ranks = bt_second_ranks_[c];
    auto& first_cumulated_ranks = bt_first_level_cumulated_ranks_[c];
    //auto first_ranks = bt_first_ranks_[c];

    int current_block = i/first_level_length_;
    int current_length = first_level_length_;

    i -= current_block*current_length;
    int level = 0;

    int r = (current_block == 0) ? 0 : (*first_cumulated_ranks)[current_block-1];
    while (level < number_of_levels_-1) {
        if ((*bt_bv_[level])[current_block]) { // Case InternalBlock
            current_length /= r_;
            int child_number = i/current_length;
            i -= child_number*current_length;
            current_block = (*bt_bv_rank_[level])(current_block)*r_ + child_number;
            ++level;
            r += (current_block%r_ == 0) ? 0 : (*cumulated_ranks[level-1])[current_block-1];
        } else { // Case BackBlock
            int index = current_block - (*bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*bt_offsets_[level])[index];
            current_block = encoded_offset/current_length;

            i += encoded_offset%current_length;
            r += (*second_ranks[level])[index];
            if (i  >= current_length) {

                ++current_block;
                i -= current_length;
            } else {
                if (level != 0 )r -= ((current_block%r_ == 0) ?  ((*cumulated_ranks[level-1])[current_block]) :  (((*cumulated_ranks[level-1])[current_block]) - ((*cumulated_ranks[level-1])[current_block-1])) );
                else r -= ((current_block == 0) ?  ((*first_cumulated_ranks)[current_block]) :  (((*first_cumulated_ranks)[current_block]) - ((*first_cumulated_ranks)[current_block-1])) );
            }

        }
    }

    i  += current_block*current_length;
    int d = mapping_[c];
    for (int j = current_block*current_length; j <= i; ++j) {
        if ((*leaf_string_)[j] == d) ++r;
    }

    return r;
    /*
   int qq = (onesRank*max_length_level + zerosRank*(max_length_level-1));
   int rr = (onesRank*max_length_level + zerosRank*(max_length_level-1)) + i;

   return r + leaf_string_wt_->rank(rr+1, c) - leaf_string_wt_->rank(qq, c);
    */
}

int PCBlockTree::better_select(int c, int k) {

    auto& cumulated_ranks = bt_cumulated_ranks_[c];
    auto& second_ranks = bt_second_ranks_[c];
    //auto first_ranks = bt_first_ranks_[c];
    auto& first_level_cumulated_ranks = bt_first_level_cumulated_ranks_[c];

    int current_block = (k-1)/first_level_length_;

    int end_block = number_of_blocks_first_level_-1;
    while (current_block != end_block) {
        int m = current_block + (end_block-current_block)/2;
        int f = (*first_level_cumulated_ranks)[m];
        if (f < k) {
            /*
            if (end_block - initial_block == 1) {
                if ((*first_level_prefix_ranks)[m+1] < k) {
                    initial_block = m+1;
                }
                break;
            }
            initial_block = m;
             */
            current_block = m+1;
        } else {
            end_block = m;

        }
    }
    /* // Change for first_level_prefix_ranks
    while (initial_block < bt_bv_[0]->size()-1 && k > (*bt_prefix_ranks_[c][0])[initial_block+1]) {
        ++initial_block;
    }
     */

    int s = current_block*first_level_length_;
    k -= (current_block == 0) ? 0 : (*first_level_cumulated_ranks)[current_block-1];


    int current_length = first_level_length_;
    int level = 0;
    while (level < number_of_levels_-1) {
        if ((*bt_bv_[level])[current_block]) { // Case InternalBlock
            int firstChild = (*bt_bv_rank_[level])(current_block)*r_;
            int child = firstChild;
            /* // Fix this to counter - 1, because of firstlevelprefixranks_ , and factorized map
            int first = firstChild;
            int last = firstChild + r_ - 1;

            while (true) {
                int m = first + (last-first)/2;
                if (m == firstChild+r_ - 1) {
                    child = m;
                    break;
                }
                int f = (*bt_prefix_ranks_[c][counter+1])[m];
                int s = (*bt_prefix_ranks_[c][counter+1])[m+1];
                if (f<k && s >= k) {
                    child = m;
                    break;
                }

                if (f >= k) {
                    last = m-1;
                } else {
                    first = m+1;
                }
            }
            */

            /* //Fix this
            int end_child = firstChild + r_-1;
            while (child != end_child) {
                int m = child + (end_child-child)/2;
                int f = (*prefix_ranks[counter+1-1])[m];
                if (f < k) {
                    if (end_child - child == 1) {
                        if ((*prefix_ranks[counter+1-1])[m+1] < k) {
                            child = m+1;
                        }
                        break;
                    }
                    child = m;
                } else {
                    end_child = m-1;
                }
            }
            */
            // Sequential search behaves better
            int last_possible_child = (firstChild + r_-1 > (*cumulated_ranks[level+1-1]).size()-1) ?  (*cumulated_ranks[level+1-1]).size()-1 : firstChild + r_-1;
            while ( child < last_possible_child && k > (*cumulated_ranks[level+1-1])[child]) {
                ++child;
            }

            k -= (child%r_ == 0) ? 0 : (*cumulated_ranks[level+1-1])[child-1];
            current_length /= r_;
            s += (child-firstChild)*current_length;
            current_block = child;
            ++level;
        } else { // Case BackBlock
            int index = current_block - (*bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*bt_offsets_[level])[index];
            current_block = encoded_offset/current_length;
            s -= encoded_offset%current_length;
            k -= (*second_ranks[level])[index];
            if (k > 0) {
                s += current_length;
                ++current_block;
            } else {
                if (level != 0 )k += ((current_block%r_ == 0) ?  ((*cumulated_ranks[level-1])[current_block]) :  (((*cumulated_ranks[level-1])[current_block]) - ((*cumulated_ranks[level-1])[current_block-1])));
                else k += ((current_block == 0) ?  ((*first_level_cumulated_ranks)[current_block]) :  (((*first_level_cumulated_ranks)[current_block]) - ((*first_level_cumulated_ranks)[current_block-1])));
            }

        }
    }

    int d = mapping_[c];
    for (int j = current_block*current_length;  ; ++j) {
        if ((*leaf_string_)[j] == d) --k;
        if (!k) return s + j - current_block*current_length;
    }
/*
    int qq = (onesRank*max_length_level + zerosRank*(max_length_level-1));
    return s+leaf_string_wt_->select(k + leaf_string_wt_->rank(qq, c), c)-qq;
*/
    return -1;
}


void PCBlockTree::print_statistics() {
    int leaf_string_size = sdsl::size_in_bytes(*leaf_string_);
    int c_leaf_string_size = sdsl::size_in_bytes(*c_leaf_string_);
//    int wt_leaf_string_size = sdsl::size_in_bytes(*leaf_string_wt_);

    int alphabet_size = sdsl::size_in_bytes(*alphabet_);
    int mapping_size = sizeof(int) * 256;

    int leaf_bv_size = sdsl::size_in_bytes(*leaf_bv_);
    int leaf_bv_rank_size = sdsl::size_in_bytes(*leaf_bv_rank_);
    int c_leaf_bv_size = sdsl::size_in_bytes(*c_leaf_bv_);
    int c_leaf_bv_rank_size = sdsl::size_in_bytes(*c_leaf_bv_rank_);


    int bt_bv_size = sizeof(void*);
    for (sdsl::bit_vector* bv : bt_bv_) {
        bt_bv_size += sdsl::size_in_bytes(*bv);
    }
    int c_bt_bv_size = sizeof(void*);
    for (sdsl::rrr_vector<63>* bv : c_bt_bv_) {
        c_bt_bv_size += sdsl::size_in_bytes(*bv);
    }


    int bt_bv_rank_size = sizeof(void*);
    for (sdsl::rank_support_v<1>* bvr : bt_bv_rank_) {
        bt_bv_rank_size += sdsl::size_in_bytes(*bvr);
    }
    int c_bt_bv_rank_size = sizeof(void*);
    for (sdsl::rrr_vector<63>::rank_1_type* bvr : c_bt_bv_rank_) {
        c_bt_bv_rank_size += sdsl::size_in_bytes(*bvr);
    }


    int bt_offsets_size = sizeof(void*);
    for (sdsl::int_vector<>* offsets: bt_offsets_) {
        bt_offsets_size += sdsl::size_in_bytes(*offsets);
    }
    int c_bt_offsets_size = sizeof(void*);
    for (sdsl::dac_vector_dp<sdsl::rrr_vector<>>* offsets: c_bt_offsets_) {
        c_bt_offsets_size += sdsl::size_in_bytes(*offsets);
    }



int bt_ranks_total_size = (bt_ranks_.size()+1) * sizeof(void*);
    int bt_ranks_first_level_size = 0;
    int bt_last_level_ranks_total_size = 0;
    for (auto pair: bt_ranks_) {
        bt_last_level_ranks_total_size += sdsl::size_in_bytes(*(bt_ranks_[pair.first].back()));
        bt_ranks_first_level_size += sdsl::size_in_bytes(*(bt_ranks_[pair.first][0]));
        int size = 0;
        for (sdsl::int_vector<>* ranks: pair.second) {
            size += sdsl::size_in_bytes(*ranks);
        }
        bt_ranks_total_size += size;
    }
    int c_bt_ranks_total_size = (c_bt_ranks_.size()+1) * sizeof(void*);
    int c_bt_ranks_first_level_size = 0;
    int c_bt_last_level_ranks_total_size = 0;
    for (auto pair: c_bt_ranks_) {
        c_bt_last_level_ranks_total_size += sdsl::size_in_bytes(*(c_bt_ranks_[pair.first].back()));
        c_bt_ranks_first_level_size += sdsl::size_in_bytes(*(c_bt_ranks_[pair.first][0]));
        int size = 0;
        for (sdsl::dac_vector_dp<sdsl::rrr_vector<>>* ranks: pair.second) {
            size += sdsl::size_in_bytes(*ranks);
        }
        c_bt_ranks_total_size += size;
    }


    int bt_prefix_ranks_total_size = (bt_prefix_ranks_.size()+1) * sizeof(void*);
    int bt_prefix_ranks_first_level_size = 0;
    for (auto pair: bt_first_level_prefix_ranks_) {
        bt_prefix_ranks_first_level_size += sdsl::size_in_bytes(*(pair.second));
    }
    int bt_last_level_prefix_ranks_total_size = 0;
    for (auto pair: bt_prefix_ranks_) {
        bt_last_level_prefix_ranks_total_size += sdsl::size_in_bytes(*(bt_prefix_ranks_[pair.first].back()));
        int size = 0;
        for (sdsl::int_vector<>* ranks: pair.second) {
            size += sdsl::size_in_bytes(*ranks);
        }
        bt_prefix_ranks_total_size += size;
    }
    int c_bt_prefix_ranks_total_size = (c_bt_prefix_ranks_.size()+1) * sizeof(void*);
    int c_bt_prefix_ranks_first_level_size = 0;
    for (auto pair: c_bt_first_level_prefix_ranks_) {
        c_bt_prefix_ranks_first_level_size += sdsl::size_in_bytes(*(pair.second));
    }
    int c_bt_last_level_prefix_ranks_total_size = 0;
    for (auto pair: c_bt_prefix_ranks_) {
        c_bt_last_level_prefix_ranks_total_size += sdsl::size_in_bytes(*(c_bt_prefix_ranks_[pair.first].back()));
        int size = 0;
        for (sdsl::dac_vector_dp<sdsl::rrr_vector<>>* ranks: pair.second) {
            size += sdsl::size_in_bytes(*ranks);
        }
        c_bt_prefix_ranks_total_size += size;
    }



    int bt_better_prefix_ranks_total_size = (bt_better_prefix_ranks_.size()+1) * sizeof(void*);
    for (auto pair: bt_better_prefix_ranks_) {
        int size = 0;
        for (sdsl::int_vector<>* ranks: pair.second) {
            size += sdsl::size_in_bytes(*ranks);
        }
        bt_better_prefix_ranks_total_size += size;
    }

    int c_bt_better_prefix_ranks_total_size = (c_bt_better_prefix_ranks_.size()+1) * sizeof(void*);
    for (auto pair: c_bt_better_prefix_ranks_) {
        int size = 0;
        for (sdsl::dac_vector_dp<sdsl::rrr_vector<>>* ranks: pair.second) {
            size += sdsl::size_in_bytes(*ranks);
        }
        c_bt_better_prefix_ranks_total_size += size;
    }

    int bt_cumulated_ranks_total_size = (bt_cumulated_ranks_.size()+1) * sizeof(void*);
    int bt_cumulated_ranks_first_level_size = 0;
    for (auto pair: bt_first_level_cumulated_ranks_) {
        bt_cumulated_ranks_first_level_size += sdsl::size_in_bytes(*(pair.second));
    }
    int bt_last_level_cumulated_ranks_total_size = 0;
    for (auto pair: bt_cumulated_ranks_) {
        bt_last_level_cumulated_ranks_total_size += sdsl::size_in_bytes(*(bt_cumulated_ranks_[pair.first].back()));
        int size = 0;
        for (sdsl::int_vector<>* ranks: pair.second) {
            size += sdsl::size_in_bytes(*ranks);
        }
        bt_cumulated_ranks_total_size += size;
    }
    int c_bt_cumulated_ranks_total_size = (c_bt_cumulated_ranks_.size()+1) * sizeof(void*);
    int c_bt_cumulated_ranks_first_level_size = 0;
    for (auto pair: c_bt_first_level_cumulated_ranks_) {
        c_bt_cumulated_ranks_first_level_size += sdsl::size_in_bytes(*(pair.second));
    }
    int c_bt_last_level_cumulated_ranks_total_size = 0;
    for (auto pair: c_bt_cumulated_ranks_) {
        c_bt_last_level_cumulated_ranks_total_size += sdsl::size_in_bytes(*(c_bt_cumulated_ranks_[pair.first].back()));
        int size = 0;
        for (sdsl::dac_vector_dp<sdsl::rrr_vector<>>* ranks: pair.second) {
            size += sdsl::size_in_bytes(*ranks);
        }
        c_bt_cumulated_ranks_total_size += size;
    }


    int bt_first_ranks_total_size = (bt_first_ranks_.size()+1) * sizeof(void*);
    for (auto pair: bt_first_ranks_) {
        int size = 0;
        for (sdsl::int_vector<>* ranks: pair.second) {
            size += sdsl::size_in_bytes(*ranks);
        }
        bt_first_ranks_total_size += size;
    }
    int c_bt_first_ranks_total_size = (c_bt_first_ranks_.size()+1) * sizeof(void*);
    for (auto pair: c_bt_first_ranks_) {
        int size = 0;
        for (sdsl::dac_vector_dp<sdsl::rrr_vector<>>* ranks: pair.second) {
            size += sdsl::size_in_bytes(*ranks);
        }
        c_bt_first_ranks_total_size += size;
    }


    int bt_second_ranks_total_size = (bt_second_ranks_.size()+1) * sizeof(void*);
    for (auto pair: bt_second_ranks_) {
        int size = 0;
        for (sdsl::int_vector<>* ranks: pair.second) {
            size += sdsl::size_in_bytes(*ranks);
        }
        bt_second_ranks_total_size += size;
    }
    int c_bt_second_ranks_total_size = (c_bt_second_ranks_.size()+1) * sizeof(void*);
    for (auto pair: c_bt_second_ranks_) {
        int size = 0;
        for (sdsl::dac_vector_dp<sdsl::rrr_vector<>>* ranks: pair.second) {
            size += sdsl::size_in_bytes(*ranks);
        }
        c_bt_second_ranks_total_size += size;
    }


    int partial_total_size = mapping_size + alphabet_size + bt_bv_size+ bt_bv_rank_size+ bt_offsets_size + leaf_string_size;
    int c_partial_total_size = mapping_size + alphabet_size + c_bt_bv_size+ c_bt_bv_rank_size+ c_bt_offsets_size + c_leaf_string_size;

    std::cout << "Bit Vectors Size(B): " << bt_bv_size << std::endl;
    std::cout << "Bit Vectors Rank Size(B): " << bt_bv_rank_size << std::endl;
    std::cout << "Offsets Size Plain(B): " << bt_offsets_size << std::endl;
    std::cout << "Leaves String Size(B): " << leaf_string_size << std::endl;
    std::cout << "Mapping Size(B): " << mapping_size << std::endl;
    std::cout << "Alphabet Size(B): " << alphabet_size << std::endl;
    std::cout << "Leaves Bitvector Size(B): " << leaf_bv_size << std::endl;
    std::cout << "Leaves Bitvector Rank Size(B): " << leaf_bv_rank_size << std::endl;
    std::cout << "Partial Total Size(B): " << partial_total_size << std::endl;


    std::cout << "Compressed Bit Vectors Size(B): " << c_bt_bv_size << std::endl;
    std::cout << "Compressed Bit Vectors Rank Size(B): " << c_bt_bv_rank_size << std::endl;
    std::cout << "Compressed Offsets Size Plain(B): " << c_bt_offsets_size << std::endl;
    std::cout << "Compressed Leaves String Size(B): " << c_leaf_string_size << std::endl;
    std::cout << "Mapping Size(B): " << mapping_size << std::endl;
    std::cout << "Alphabet Size(B): " << alphabet_size << std::endl;
    std::cout << "Compressed Leaves Bitvector Size(B): " << c_leaf_bv_size << std::endl;
    std::cout << "Compressed Leaves Bitvector Rank Size(B): " << c_leaf_bv_rank_size << std::endl;
    std::cout << "Compressed Partial Total Size(B): " << c_partial_total_size << std::endl;
//    std::cout << "WT Leaves String Size(B):" << wt_leaf_string_size << std::endl;


    std::cout << std::endl << "RANK" << std::endl;
    std::cout << "first_level_prefix_ranks Plain(B): " << bt_prefix_ranks_first_level_size << std::endl;
    std::cout << "first_level_cumulated_ranks Plain(B): " << bt_cumulated_ranks_first_level_size << std::endl;
    std::cout << "first_level_ranks Plain(B): " << bt_ranks_first_level_size << std::endl;
    std::cout << "ranks Size Plain(B): " << bt_ranks_total_size << std::endl;
    std::cout << "prefix_ranks Size Plain(B): " << bt_prefix_ranks_total_size << std::endl;
    std::cout << "better_prefix_ranks Size Plain(B): " << bt_better_prefix_ranks_total_size << std::endl;
    std::cout << "cumulated_ranks Size Plain(B): " << bt_cumulated_ranks_total_size << std::endl;
    std::cout << "first_ranks Size Plain(B): " << bt_first_ranks_total_size << std::endl;
    std::cout << "second_ranks Size Plain(B): " << bt_second_ranks_total_size << std::endl << std::endl;
    std::cout << "Last Level ranks Size Plain(B): " << bt_last_level_ranks_total_size << std::endl;
    std::cout << "Last Level prefix_ranks Size Plain(B): " << bt_last_level_prefix_ranks_total_size << std::endl;
    std::cout << "Last Level cumulated_ranks Size Plain(B): " << bt_last_level_cumulated_ranks_total_size << std::endl;

    std::cout << std::endl << "Compressed RANK" << std::endl;
    std::cout << "Compressed first_level_prefix_ranks(B): " << c_bt_prefix_ranks_first_level_size << std::endl;
    std::cout << "Compressed first_level_cumulated_ranks(B): " << c_bt_cumulated_ranks_first_level_size << std::endl;
    std::cout << "Compressed first_level_ranks Plain(B): " << c_bt_ranks_first_level_size << std::endl;
    std::cout << "Compressed ranks Size(B): " << c_bt_ranks_total_size << std::endl;
    std::cout << "Compressed prefix_ranks Size(B): " << c_bt_prefix_ranks_total_size << std::endl;
    std::cout << "Compressed better_prefix_ranks Size(B): " << c_bt_better_prefix_ranks_total_size << std::endl;
    std::cout << "Compressed cumulated_ranks Size(B): " << c_bt_cumulated_ranks_total_size << std::endl;
    std::cout << "Compressed first_ranks Size(B): " << c_bt_first_ranks_total_size << std::endl;
    std::cout << "Compressed second_ranks Size(B): " << c_bt_second_ranks_total_size << std::endl << std::endl;
    std::cout << "Compressed Last Level ranks Size(B): " << c_bt_last_level_ranks_total_size << std::endl;
    std::cout << "Compressed Last Level prefix_ranks Size(B): " << c_bt_last_level_prefix_ranks_total_size << std::endl;
    std::cout << "Compressed Last Level cumulated_ranks Size(B): " << c_bt_last_level_cumulated_ranks_total_size << std::endl <<std::endl;


    int rank_paper_version_size = bt_prefix_ranks_first_level_size + bt_first_ranks_total_size + bt_second_ranks_total_size + bt_prefix_ranks_total_size;
    int rank_better_paper_version_size = bt_cumulated_ranks_first_level_size + bt_second_ranks_total_size + bt_better_prefix_ranks_total_size;
    int rank_alternative_version_size = bt_second_ranks_total_size + bt_ranks_total_size + bt_prefix_ranks_first_level_size;
    int rank_better_version_size = bt_second_ranks_total_size + bt_cumulated_ranks_first_level_size + bt_cumulated_ranks_total_size;
    std::cout << "RANK Paper Version Size(B): " << rank_paper_version_size << std::endl;
    std::cout << "RANK Better_Paper Version Size(B): " << rank_better_paper_version_size << std::endl;
    std::cout << "RANK Alternative Version Size(B): " << rank_alternative_version_size << std::endl;
    std::cout << "RANK Better Version Size(B): " << rank_better_version_size << std::endl << std::endl;

    int c_rank_paper_version_size = c_bt_prefix_ranks_first_level_size + c_bt_first_ranks_total_size + c_bt_second_ranks_total_size + c_bt_prefix_ranks_total_size;
    int c_rank_better_paper_version_size = c_bt_cumulated_ranks_first_level_size + c_bt_second_ranks_total_size + c_bt_better_prefix_ranks_total_size;
    int c_rank_alternative_version_size = c_bt_second_ranks_total_size + c_bt_ranks_total_size + c_bt_prefix_ranks_first_level_size;
    int c_rank_better_version_size = c_bt_second_ranks_total_size + c_bt_cumulated_ranks_first_level_size + c_bt_cumulated_ranks_total_size;
    std::cout << "Compressed RANK Paper Version Size(B): " << c_rank_paper_version_size << std::endl;
    std::cout << "Compressed RANK Better Paper Version Size(B): " << c_rank_better_paper_version_size << std::endl;
    std::cout << "Compressed RANK Alternative Version Size(B): " << c_rank_alternative_version_size << std::endl;
    std::cout << "Compressed RANK Better Version Size(B): " << c_rank_better_version_size << std::endl << std::endl;

    std::cout << "TOTAL" << std::endl;
    std::cout << "Total Size (with paper rank)(B): " << rank_paper_version_size + partial_total_size << std::endl;
    std::cout << "Total Size (with better_paper rank)(B): " << rank_better_paper_version_size + partial_total_size << std::endl;
    std::cout << "Total Size (with alternative rank)(B): " << rank_alternative_version_size + partial_total_size  << std::endl;
    std::cout << "Total Size (with better rank)(B): " << rank_better_version_size + partial_total_size << std::endl << std::endl;

    std::cout << "Compressed WITHOUT RANK COMPRESSION" << std::endl;
    std::cout << "Compressed Total Size (with paper rank)(B): " << rank_paper_version_size + c_partial_total_size << std::endl;
    std::cout << "Compressed Total Size (with better_paper rank)(B): " << rank_better_paper_version_size + c_partial_total_size << std::endl;
    std::cout << "Compressed Total Size (with alternative rank)(B): " << rank_alternative_version_size + c_partial_total_size << std::endl;
    std::cout << "Compressed Total Size (with better rank)(B): " << rank_better_version_size + c_partial_total_size << std::endl << std::endl;

    std::cout << "Compressed TOTAL" << std::endl;
    std::cout << "Compressed Total Size (with paper rank)(B): " << c_rank_paper_version_size + c_partial_total_size << std::endl;
    std::cout << "Compressed Total Size (with better_paper rank)(B): " << c_rank_better_paper_version_size + c_partial_total_size << std::endl;
    std::cout << "Compressed Total Size (with alternative rank)(B): " << c_rank_alternative_version_size + c_partial_total_size << std::endl;
    std::cout << "Compressed Total Size (with better rank)(B): " << c_rank_better_version_size + c_partial_total_size << std::endl << std::endl;

/*
    std::cout << "PERFORMANCE" << std::endl;


    bool ok = true;
    struct rusage usage;
    struct timeval ru_start, rs_start, ru_end, rs_end;
    getrusage(RUSAGE_SELF, &usage);
    rs_start = usage.ru_stime;
    ru_start = usage.ru_utime;

    for (int i = 0; i < input_.size(); ++i) {
        ok = ok && (access(i) == ((int)input_[i]));
    }
    if (!ok) std::cout << "Error" << std::endl;

    getrusage(RUSAGE_SELF, &usage);
    rs_end = usage.ru_stime;
    ru_end = usage.ru_utime;
    int user_time = (ru_end.tv_sec - ru_start.tv_sec)*1000000 + (ru_end.tv_usec - ru_start.tv_usec);
    int system_time = (rs_end.tv_sec - rs_start.tv_sec)*1000000 + (rs_end.tv_usec - rs_start.tv_usec);

    std::cout << "Total Access Time(us): " << user_time+system_time << std::endl;



    std::unordered_map<int,int> ranks;
    for (auto pair : bt_ranks_)
        ranks[pair.first] = 0;

    ok = true;
    getrusage(RUSAGE_SELF, &usage);
    rs_start = usage.ru_stime;
    ru_start = usage.ru_utime;
    for (int i = 0; i < input_.size(); ++i) {
        ranks[input_[i]] = ranks[input_[i]] + 1;
        ok = ok && (rank(input_[i], i) == ranks[input_[i]]);
    }
    if (!ok) std::cout << "Error" << std::endl;

    getrusage(RUSAGE_SELF, &usage);
    rs_end = usage.ru_stime;
    ru_end = usage.ru_utime;
    user_time = (ru_end.tv_sec - ru_start.tv_sec)*1000000 + (ru_end.tv_usec - ru_start.tv_usec);
    system_time = (rs_end.tv_sec - rs_start.tv_sec)*1000000 + (rs_end.tv_usec - rs_start.tv_usec);

    std::cout << "Total rank Time (us): " << user_time+system_time << std::endl;



    for (auto pair : bt_ranks_)
        ranks[pair.first] = 0;

    ok = true;
    getrusage(RUSAGE_SELF, &usage);
    rs_start = usage.ru_stime;
    ru_start = usage.ru_utime;

    for (int i = 0; i<input_.size(); ++i) {
        ranks[input_[i]] = ranks[input_[i]] + 1;
        ok = ok && (select(input_[i], ranks[input_[i]]) == i);
    }
    if (!ok) std::cout << "Error" << std::endl;

    getrusage(RUSAGE_SELF, &usage);
    rs_end = usage.ru_stime;
    ru_end = usage.ru_utime;
    user_time = (ru_end.tv_sec - ru_start.tv_sec)*1000000 + (ru_end.tv_usec - ru_start.tv_usec);
    system_time = (rs_end.tv_sec - rs_start.tv_sec)*1000000 + (rs_end.tv_usec - rs_start.tv_usec);

    std::cout << "Total select Time (us): " << user_time+system_time << std::endl;




    for (auto pair : bt_ranks_)
        ranks[pair.first] = 0;

    ok = true;
    getrusage(RUSAGE_SELF, &usage);
    rs_start = usage.ru_stime;
    ru_start = usage.ru_utime;
    for (int i = 0; i < input_.size(); ++i) {
        ranks[input_[i]] = ranks[input_[i]] + 1;
        ok = ok && (rank_alternative(input_[i], i) == ranks[input_[i]]);
    }
    if (!ok) std::cout << "Error" << std::endl;

    getrusage(RUSAGE_SELF, &usage);
    rs_end = usage.ru_stime;
    ru_end = usage.ru_utime;
    user_time = (ru_end.tv_sec - ru_start.tv_sec)*1000000 + (ru_end.tv_usec - ru_start.tv_usec);
    system_time = (rs_end.tv_sec - rs_start.tv_sec)*1000000 + (rs_end.tv_usec - rs_start.tv_usec);

    std::cout << "Total rank_alternative Time (us): " << user_time+system_time << std::endl;



    for (auto pair : bt_ranks_)
        ranks[pair.first] = 0;

    ok = true;
    getrusage(RUSAGE_SELF, &usage);
    rs_start = usage.ru_stime;
    ru_start = usage.ru_utime;

    for (int i = 0; i<input_.size(); ++i) {
        ranks[input_[i]] = ranks[input_[i]] + 1;
        ok = ok && (select_alternative(input_[i], ranks[input_[i]]) == i);
    }
    if (!ok) std::cout << "Error" << std::endl;

    getrusage(RUSAGE_SELF, &usage);
    rs_end = usage.ru_stime;
    ru_end = usage.ru_utime;
    user_time = (ru_end.tv_sec - ru_start.tv_sec)*1000000 + (ru_end.tv_usec - ru_start.tv_usec);
    system_time = (rs_end.tv_sec - rs_start.tv_sec)*1000000 + (rs_end.tv_usec - rs_start.tv_usec);

    std::cout << "Total select_alternative Time (us): " << user_time+system_time << std::endl;


    for (auto pair : bt_ranks_)
        ranks[pair.first] = 0;

    ok = true;
    getrusage(RUSAGE_SELF, &usage);
    rs_start = usage.ru_stime;
    ru_start = usage.ru_utime;
    for (int i = 0; i < input_.size(); ++i) {
        ranks[input_[i]] = ranks[input_[i]] + 1;
        ok = ok && (better_rank(input_[i], i) == ranks[input_[i]]);
    }
    if (!ok) std::cout << "Error" << std::endl;

    getrusage(RUSAGE_SELF, &usage);
    rs_end = usage.ru_stime;
    ru_end = usage.ru_utime;
    user_time = (ru_end.tv_sec - ru_start.tv_sec)*1000000 + (ru_end.tv_usec - ru_start.tv_usec);
    system_time = (rs_end.tv_sec - rs_start.tv_sec)*1000000 + (rs_end.tv_usec - rs_start.tv_usec);

    std::cout << "Total better_rank Time (us): " << user_time+system_time << std::endl;



    for (auto pair : bt_ranks_)
        ranks[pair.first] = 0;

    ok = true;
    getrusage(RUSAGE_SELF, &usage);
    rs_start = usage.ru_stime;
    ru_start = usage.ru_utime;

    for (int i = 0; i<input_.size(); ++i) {
        ranks[input_[i]] = ranks[input_[i]] + 1;
        ok = ok && (better_select(input_[i], ranks[input_[i]]) == i);
    }
    if (!ok) std::cout << "Error" << std::endl;

    getrusage(RUSAGE_SELF, &usage);
    rs_end = usage.ru_stime;
    ru_end = usage.ru_utime;
    user_time = (ru_end.tv_sec - ru_start.tv_sec)*1000000 + (ru_end.tv_usec - ru_start.tv_usec);
    system_time = (rs_end.tv_sec - rs_start.tv_sec)*1000000 + (rs_end.tv_usec - rs_start.tv_usec);

    std::cout << "Total better_select Time (us): " << user_time+system_time << std::endl;
*/
}

