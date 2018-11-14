//
// Created by sking32 on 4/17/18.
//

#include <compressed/CBlockTree.h>
#include <iostream>
#include <unordered_set>

CBlockTree::CBlockTree(BlockTree * bt) : r_(bt->r_), max_leaf_length_(bt->max_leaf_length_), input_(bt->input_) {
    std::vector<Block*> first_level = {bt->root_block_};
    bool is_first_level = false;
    while (!is_first_level) {
        for (Block* b: first_level) {
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

    max_length_first_level_ = first_level[0]->length();
    number_of_blocks_first_level_ = first_level.size();
    number_of_levels_ = 0;

    std::vector<Block*> current_level = first_level;
    std::vector<Block*> next_level = bt->next_level(first_level);

    while (next_level.size() != 0) {
        sdsl::bit_vector* current_level_bv = new sdsl::bit_vector(current_level.size() ,0);
        sdsl::bit_vector* current_level_bv_block_length = new sdsl::bit_vector(current_level.size() ,0);

        std::unordered_map<int,sdsl::int_vector<>*> next_level_ranks;
        std::unordered_map<int,sdsl::int_vector<>*> next_level_prefix_ranks;
        std::unordered_map<int,sdsl::int_vector<>*> next_level_cumulated_ranks;

        for (auto pair: next_level[0]->ranks_) { // Case root_block (?)
            next_level_ranks[pair.first] = new sdsl::int_vector<>(next_level.size());
        }
        for (auto pair: next_level[0]->prefix_ranks_) { // Case root_block (?)
            next_level_prefix_ranks[pair.first] = new sdsl::int_vector<>(next_level.size());
        }
        for (auto pair: next_level[0]->cumulated_ranks_) { // Case root_block (?)
            next_level_cumulated_ranks[pair.first] = new sdsl::int_vector<>(next_level.size());
        }

        int number_of_leaves = 0;
        int max_block_length = current_level[0]->length();
        for (int i = 0; i < current_level.size(); ++i) {
            if (current_level[i]->length() > max_block_length) max_block_length = current_level[i]->length();
        }


        for (int i = 0; i < current_level.size(); ++i) {
            current_level[i]->level_index_ = i;

            if (current_level[i]->length() < max_block_length) {
                (*current_level_bv_block_length)[i] = 0;
            } else {
                (*current_level_bv_block_length)[i] = 1;
            }


            if (current_level[i]->is_leaf()) {
                (*current_level_bv)[i] = 0;
                ++number_of_leaves;
            }
            else {
                (*current_level_bv)[i] = 1;
            }
        }

        for (int i = 0; i < next_level.size(); ++i) {
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

                (*current_level_offsets)[j++] = current_level[i]->first_block_->level_index_ * max_block_length + current_level[i]->offset_;
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

        bt_bv_block_length_.push_back(current_level_bv_block_length);
        sdsl::rank_support_v<1>* current_level_bv_block_length_rank = new sdsl::rank_support_v<1>(current_level_bv_block_length);
        bt_bv_block_length_rank_.push_back(current_level_bv_block_length_rank);

        c_bt_bv_.push_back(new sdsl::rrr_vector<63>(*bt_bv_.back()));
        c_bt_bv_rank_.push_back(new sdsl::rrr_vector<63>::rank_1_type(c_bt_bv_.back()));
        c_bt_bv_block_length_.push_back(new sdsl::rrr_vector<63>(*bt_bv_block_length_.back()));
        c_bt_bv_block_length_rank_.push_back(new sdsl::rrr_vector<63>::rank_1_type(c_bt_bv_block_length_.back()));


        current_level = next_level;
        next_level = bt->next_level(current_level);
        ++number_of_levels_;
    }

    ++number_of_levels_;

    std::vector<Block*> last_level = current_level;
    int max_block_length = last_level[0]->length();
    for (int i = 0; i < last_level.size(); ++i) {
        if (max_block_length < last_level[i]->length()) max_block_length = last_level[i]->length();
    }

    sdsl::bit_vector* last_level_bv_block_length = new sdsl::bit_vector(last_level.size() ,0);

    for (int i = 0; i < last_level.size(); ++i) {
        Block* b = last_level[i];
        if (b->length() == max_block_length) {
            (*last_level_bv_block_length)[i] = 1;
        }
        else {
            (*last_level_bv_block_length)[i] = 0;
        }
    }

    bt_bv_block_length_.push_back(last_level_bv_block_length);
    sdsl::rank_support_v<1>* last_level_bv_block_length_rank = new sdsl::rank_support_v<1>(last_level_bv_block_length);
    bt_bv_block_length_rank_.push_back(last_level_bv_block_length_rank);

    c_bt_bv_block_length_.push_back(new sdsl::rrr_vector<63>(*bt_bv_block_length_.back()));
    c_bt_bv_block_length_rank_.push_back(new sdsl::rrr_vector<63>::rank_1_type(c_bt_bv_block_length_.back()));

    std::string leaf_string = "";
    for (Block* b: current_level) {
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

CBlockTree::~CBlockTree() {

    for (sdsl::bit_vector* bv : bt_bv_)
        delete bv;
    for (sdsl::rank_support_v<1>* rank : bt_bv_rank_)
        delete rank;

    for (sdsl::bit_vector* bv : bt_bv_block_length_)
        delete bv;
    for (sdsl::rank_support_v<1>* rank : bt_bv_block_length_rank_)
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

    for (sdsl::rrr_vector<63>* bv : c_bt_bv_block_length_)
        delete bv;
    for (sdsl::rrr_vector<63>::rank_1_type* rank : c_bt_bv_block_length_rank_)
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

int CBlockTree::access(int i) {
    /*
    int p = number_of_blocks_first_level_/r_;
    int current_length = n_;
    int initial_block = 0;
    while (p != 0) {
        int max_length_child = (current_length % r_ == 0) ? current_length / r_ : current_length / r_ + 1;
        int big_blocks = (current_length % r_ == 0) ? r_ : current_length % r_;
        if (i < max_length_child * big_blocks) {
            int child_number = i / max_length_child;
            current_length = max_length_child;
            i = i - child_number * max_length_child;
            initial_block += child_number*p;
        } else {
            i = i - max_length_child * big_blocks;
            int child_number = big_blocks + i / (max_length_child - 1);
            current_length = max_length_child - 1;
            i = i - (child_number - big_blocks) * (max_length_child - 1);
            initial_block += child_number*p;
        }
        p /= r_;
    }
    */
    int current_block = i/max_length_first_level_;
    int onesRank = (*(bt_bv_block_length_rank_[0]))(current_block+1);
    int zerosRank = current_block+1 - onesRank;
    int acc = i;
    int delta_ones = onesRank;
    int new_guess = current_block;
    while (onesRank*max_length_first_level_ + zerosRank*(max_length_first_level_ - 1) - 1 < i) {
        acc -= (new_guess+1)*max_length_first_level_ - (new_guess + 1 - delta_ones);
        new_guess = acc/max_length_first_level_;
        current_block += 1 + new_guess;
        delta_ones = -onesRank;
        onesRank = (*(bt_bv_block_length_rank_[0]))(current_block+1);
        delta_ones += onesRank;
        zerosRank = current_block+1 - onesRank;
    }
/*
    if  (j < i) {
        initial_block += (i-j-1)/max_length_first_level_;
        onesRank = (*(bt_bv_block_length_rank_[0]))(initial_block+1);
        zerosRank = initial_block+1 - onesRank;
        j = onesRank*max_length_first_level_ + zerosRank*(max_length_first_level_ - 1) - 1;
    }


    while (j < i) {
        ++initial_block;
        j += ((*(bt_bv_block_length_[0]))[initial_block]) ? max_length_first_level_ : max_length_first_level_ - 1;
    }
*/
    int current_length = ((*(bt_bv_block_length_[0]))[current_block]) ? max_length_first_level_ : max_length_first_level_ - 1;
    i -= onesRank*max_length_first_level_ + zerosRank*(max_length_first_level_ - 1) - current_length;

    int max_length_level = max_length_first_level_;
    int level = 0;

    while (level < number_of_levels_-1) {
        if ((*bt_bv_[level])[current_block]) { // Case InternalBlock
            int big_blocks = (current_length%r_ == 0) ? r_ : current_length%r_;
            current_length = (current_length%r_ == 0) ? current_length/r_ : current_length/r_ + 1;
            if (i < current_length*big_blocks) {
                int child_number = i/current_length;
                i -= child_number*current_length;
                current_block = (*bt_bv_rank_[level])(current_block)*r_ + child_number;
            } else {
                i -= current_length*big_blocks;
                --current_length;
                int child_number = big_blocks + i/current_length;
                i -= (child_number-big_blocks) * current_length;
                current_block = (*bt_bv_rank_[level])(current_block)*r_ + child_number;
            }
            max_length_level = (max_length_level%r_ == 0) ? max_length_level/r_ : max_length_level/r_ + 1;
            ++level;
        } else { // Case BackBlock
            int index = current_block - (*bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*bt_offsets_[level])[index];
            current_block = encoded_offset/max_length_level;
            current_length = ((*bt_bv_block_length_[level])[current_block]) ? max_length_level : max_length_level-1;

            i += encoded_offset%max_length_level;
            if (i >= current_length) {
                ++current_block;
                i -= current_length;
                current_length = ((*bt_bv_block_length_[level])[current_block]) ? max_length_level : max_length_level-1;
            }
        }
    }

    onesRank =  (*bt_bv_block_length_rank_[level])(current_block);
    zerosRank = current_block - onesRank;
    i  += (onesRank*max_length_level + zerosRank*(max_length_level-1));

    return (*alphabet_)[(*leaf_string_)[i]];
}






int CBlockTree::rank(int c, int i) {
    auto& prefix_ranks = bt_prefix_ranks_[c];
    auto& second_ranks = bt_second_ranks_[c];
    auto& first_ranks = bt_first_ranks_[c];

    int current_block = i/max_length_first_level_;
    int onesRank = (*(bt_bv_block_length_rank_[0]))(current_block+1);
    int zerosRank = current_block+1 - onesRank;
    int acc = i;
    int delta_ones = onesRank;
    int new_guess = current_block;
    while (onesRank*max_length_first_level_ + zerosRank*(max_length_first_level_ - 1) - 1 < i) {
        acc -= (new_guess+1)*max_length_first_level_ - (new_guess + 1 - delta_ones);
        new_guess = acc/max_length_first_level_;
        current_block += 1 + new_guess;
        delta_ones = -onesRank;
        onesRank = (*(bt_bv_block_length_rank_[0]))(current_block+1);
        delta_ones += onesRank;
        zerosRank = current_block+1 - onesRank;
    }
/*
    if  (j < i) {
        initial_block += (i-j-1)/max_length_first_level_;
        onesRank = (*(bt_bv_block_length_rank_[0]))(initial_block+1);
        zerosRank = initial_block+1 - onesRank;
        j = onesRank*max_length_first_level_ + zerosRank*(max_length_first_level_ - 1) - 1;
    }


    while (j < i) {
        ++initial_block;
        j += ((*(bt_bv_block_length_[0]))[initial_block]) ? max_length_first_level_ : max_length_first_level_ - 1;
    }
*/
    int current_length = ((*(bt_bv_block_length_[0]))[current_block]) ? max_length_first_level_ : max_length_first_level_ - 1;

    i -= onesRank*max_length_first_level_ + zerosRank*(max_length_first_level_ - 1) - current_length;
    int max_length_level = max_length_first_level_;
    int level = 0;

    int r = (*bt_first_level_prefix_ranks_[c])[current_block];

    while (level < number_of_levels_-1) {
        if ((*bt_bv_[level])[current_block]) { // Case InternalBlock
            int big_blocks = (current_length%r_ == 0) ? r_ : current_length%r_;
            current_length = (current_length%r_ == 0) ? current_length/r_ : current_length/r_ + 1;
            if (i < current_length*big_blocks) {
                int child_number = i/current_length;
                i -= child_number*current_length;
                current_block = (*bt_bv_rank_[level])(current_block)*r_ + child_number;
            } else {
                i -= current_length*big_blocks;
                --current_length;
                int child_number = big_blocks + i/current_length;
                i -= (child_number-big_blocks) * current_length;
                current_block = (*bt_bv_rank_[level])(current_block)*r_ + child_number;
            }
            ++level;
            r += (*prefix_ranks[level-1])[current_block];
            max_length_level = (max_length_level % r_) ? max_length_level/r_ +1 : max_length_level/r_;
        } else { // Case BackBlock
            int index = current_block  -(*bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*bt_offsets_[level])[index];
            current_block = encoded_offset/max_length_level;
            current_length = ((*bt_bv_block_length_[level])[current_block]) ? max_length_level : max_length_level-1;

            i += encoded_offset%max_length_level;
            if (i >= current_length) {
                r += (*second_ranks[level])[index];
                ++current_block;
                i -= current_length;
                current_length = ((*bt_bv_block_length_[level])[current_block]) ? max_length_level : max_length_level-1;
            } else {
                r -= (*first_ranks[level])[index];
            }
        }
    }

    onesRank =  (*bt_bv_block_length_rank_[level])(current_block);
    zerosRank = current_block - onesRank;

    i  += (onesRank*max_length_level + zerosRank*(max_length_level-1));
    int d = mapping_[c];
    for (int j = (onesRank*max_length_level + zerosRank*(max_length_level-1)); j <= i; ++j) {
        if ((*leaf_string_)[j] == d) ++r;
    }

    return r;
     /*
    int qq = (onesRank*max_length_level + zerosRank*(max_length_level-1));
    int rr = (onesRank*max_length_level + zerosRank*(max_length_level-1)) + i;

    return r + leaf_string_wt_->rank(rr+1, c) - leaf_string_wt_->rank(qq, c);
     */
}

int CBlockTree::select(int c, int k) {

    auto& prefix_ranks = bt_prefix_ranks_[c];
    auto& second_ranks = bt_second_ranks_[c];
    auto& first_ranks = bt_first_ranks_[c];
    auto& first_level_prefix_ranks = bt_first_level_prefix_ranks_[c];

    int current_block = (k-1)/max_length_first_level_;

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

    int onesRank = (*(bt_bv_block_length_rank_[0]))(current_block);
    int zerosRank = current_block - onesRank;
    int s = onesRank*max_length_first_level_ + zerosRank*(max_length_first_level_ - 1);
    k -= (*first_level_prefix_ranks)[current_block];


    int current_length = ((*(bt_bv_block_length_[0]))[current_block]) ? max_length_first_level_ : max_length_first_level_ - 1;
    int max_length_level = max_length_first_level_;
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
                int f = (*prefix_ranks[level+1-1])[m];
                if (f < k) {
                    if (end_child - child == 1) {
                        if ((*prefix_ranks[level+1-1])[m+1] < k) {
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
            while ( child < firstChild + r_-1 && k > (*prefix_ranks[level+1-1])[child+1]) {
                ++child;
            }

            k -= (*prefix_ranks[level+1-1])[child];

            int big_blocks = (current_length%r_ == 0) ? r_ : current_length%r_;
            current_length = (current_length%r_ == 0) ? current_length/r_ : current_length/r_ + 1;
            current_block = child;
            if (child-firstChild < big_blocks) {
                s += (child-firstChild)*current_length;
            } else {
                s += big_blocks*current_length + ((child-firstChild)-big_blocks) * (current_length-1);
                --current_length;
            }
            ++level;
            max_length_level = (max_length_level % r_) ? max_length_level/r_ +1 : max_length_level/r_;
        } else { // Case BackBlock
            int index = current_block - (*bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*bt_offsets_[level])[index];
            current_block = encoded_offset/max_length_level;
            current_length = ((*bt_bv_block_length_[level])[current_block]) ? max_length_level : max_length_level-1;

            s -= encoded_offset%max_length_level;
            int second_rank = (*second_ranks[level])[index];
            if (k > second_rank) {
                s += current_length;
                k-= second_rank;
                ++current_block;
                current_length = ((*bt_bv_block_length_[level])[current_block]) ? max_length_level : max_length_level-1;
            } else {
                k += (*first_ranks[level])[index];
            }
        }
    }

    onesRank =  (*bt_bv_block_length_rank_[level])(current_block);
    zerosRank = current_block - onesRank;

    int d = mapping_[c];
    for (int j = (onesRank*max_length_level + zerosRank*(max_length_level-1));  ; ++j) {
        if ((*leaf_string_)[j] == d) --k;
        if (!k) return s + j - (onesRank*max_length_level + zerosRank*(max_length_level-1));
    }
/*
    int qq = (onesRank*max_length_level + zerosRank*(max_length_level-1));
    return s+leaf_string_wt_->select(k + leaf_string_wt_->rank(qq, c), c)-qq;
*/
    return -1;
}


int CBlockTree::rank_alternative(int c, int i) {

    auto& ranks = bt_ranks_[c];
    auto& second_ranks = bt_second_ranks_[c];

    int current_block = i/max_length_first_level_;
    int onesRank = (*(bt_bv_block_length_rank_[0]))(current_block+1);
    int zerosRank = current_block+1 - onesRank;
    int acc = i;
    int delta_ones = onesRank;
    int new_guess = current_block;
    while (onesRank*max_length_first_level_ + zerosRank*(max_length_first_level_ - 1) - 1 < i) {
        acc -= (new_guess+1)*max_length_first_level_ - (new_guess + 1 - delta_ones);
        new_guess = acc/max_length_first_level_;
        current_block += 1 + new_guess;
        delta_ones = -onesRank;
        onesRank = (*(bt_bv_block_length_rank_[0]))(current_block+1);
        delta_ones += onesRank;
        zerosRank = current_block+1 - onesRank;
    }/*
    if  (j < i) {
        initial_block += (i-j-1)/max_length_first_level_;
        onesRank = (*(bt_bv_block_length_rank_[0]))(initial_block+1);
        zerosRank = initial_block+1 - onesRank;
        j = onesRank*max_length_first_level_ + zerosRank*(max_length_first_level_ - 1) - 1;
    }


    while (j < i) {
        ++initial_block;
        j += ((*(bt_bv_block_length_[0]))[initial_block]) ? max_length_first_level_ : max_length_first_level_ - 1;
    }
*/
    int current_length = ((*(bt_bv_block_length_[0]))[current_block]) ? max_length_first_level_ : max_length_first_level_ - 1;
    i -= onesRank*max_length_first_level_ + zerosRank*(max_length_first_level_ - 1) - current_length;

    int max_length_level = max_length_first_level_;
    int level = 0;

    int r = (*bt_first_level_prefix_ranks_[c])[current_block];

    while (level < number_of_levels_-1) {
        if ((*bt_bv_[level])[current_block]) { // Case InternalBlock
            int big_blocks = (current_length%r_ == 0) ? r_ : current_length%r_;
            current_length = (current_length%r_ == 0) ? current_length/r_ : current_length/r_ + 1;
            int child_number = 0;
            if (i < current_length*big_blocks) {
                child_number = i/current_length;
                i -= child_number*current_length;
            } else {
                i -= current_length*big_blocks;
                --current_length;
                child_number = big_blocks + i/current_length;
                i -= (child_number-big_blocks) * current_length;
            }
            int firstChild = (*bt_bv_rank_[level])(current_block)*r_;
            for (int child = firstChild; child < firstChild + child_number; ++child)
                r += (*ranks[level+1])[child];
            current_block = firstChild + child_number;
            ++level;
            max_length_level = (max_length_level % r_) ? max_length_level/r_ +1 : max_length_level/r_;
        } else { // Case BackBlock
            int index = current_block - (*bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*bt_offsets_[level])[index];
            current_block = encoded_offset/max_length_level;
            current_length = ((*bt_bv_block_length_[level])[current_block]) ? max_length_level : max_length_level-1;
            int offset = encoded_offset%max_length_level;
            if (!offset) {
                if (i >= current_length) {
                    r += (*second_ranks[level])[index];

                    ++current_block;
                    i -= current_length;
                    current_length = ((*bt_bv_block_length_[level])[current_block]) ? max_length_level :
                                     max_length_level - 1;

                }
            } else {
                r += (*second_ranks[level])[index];
                i += offset;
                if (i >= current_length) {
                    ++current_block;
                    i -= current_length;
                    current_length = ((*bt_bv_block_length_[level])[current_block]) ? max_length_level :
                                     max_length_level - 1;

                } else {
                    r -= (*ranks[level])[current_block];
                }
            }
        }
    }

    onesRank =  (*bt_bv_block_length_rank_[level])(current_block);
    zerosRank = current_block - onesRank;
    i  += (onesRank*max_length_level + zerosRank*(max_length_level-1));
    int d = mapping_[c];
    for (int j = (onesRank*max_length_level + zerosRank*(max_length_level-1)); j <= i; ++j) {
        if ((*leaf_string_)[j] == d) ++r;
    }

    return r;
}

int CBlockTree::select_alternative(int c, int k) {

    auto& ranks = bt_ranks_[c];
    auto& second_ranks = bt_second_ranks_[c];
    auto& first_level_prefix_ranks = bt_first_level_prefix_ranks_[c];


    int current_block = (k-1)/max_length_first_level_;

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

    int onesRank = (*(bt_bv_block_length_rank_[0]))(current_block);
    int zerosRank = current_block - onesRank;
    int s = onesRank*max_length_first_level_ + zerosRank*(max_length_first_level_ - 1);
    k = k-(*first_level_prefix_ranks)[current_block];


    int current_length = ((*(bt_bv_block_length_[0]))[current_block]) ? max_length_first_level_ : max_length_first_level_ - 1;
    int max_length_level = max_length_first_level_;
    int level = 0;

    while (level < number_of_levels_-1) {
        if ((*bt_bv_[level])[current_block]) { // Case InternalBlock
            int firstChild = (*bt_bv_rank_[level])(current_block)*r_;
            int child = firstChild;
            int r = (*ranks[level+1])[child];
            while ( child < firstChild + r_ && k > r) {
                ++child;
                r+= (*ranks[level+1])[child];
            }
            k -= r - (*ranks[level+1])[child];
            int big_blocks = (current_length%r_ == 0) ? r_ : current_length%r_;
            current_length = (current_length%r_ == 0) ? current_length/r_ : current_length/r_ + 1;
            current_block = child;
            if (child-firstChild < big_blocks) {
                s += (child-firstChild)*current_length;
            } else {
                s += big_blocks*current_length + ((child-firstChild)-big_blocks) * (current_length-1);
                --current_length;
            }
            ++level;
            max_length_level = (max_length_level % r_) ? max_length_level/r_ +1 : max_length_level/r_;
        } else { // Case BackBlock
            int index = current_block - (*bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*bt_offsets_[level])[index];
            current_block = encoded_offset/max_length_level;
            current_length = ((*bt_bv_block_length_[level])[current_block]) ? max_length_level : max_length_level-1;

            int offset = encoded_offset%max_length_level;
            int second_rank = (*second_ranks[level])[index];
            if (!offset) {
                if (k > second_rank) {
                    s += current_length;
                    k -= second_rank;
                    ++current_block;
                    current_length = ((*bt_bv_block_length_[level])[current_block]) ? max_length_level :
                                     max_length_level - 1;
                }
            } else {
                s -= offset;
                k -= second_rank;
                if (k > 0) {
                    s += current_length;
                    ++current_block;
                    current_length = ((*bt_bv_block_length_[level])[current_block]) ? max_length_level :
                                     max_length_level - 1;
                } else {
                    k += (*ranks[level])[current_block];
                }
            }
        }
    }

    onesRank =  (*bt_bv_block_length_rank_[level])(current_block);
    zerosRank = current_block - onesRank;

    int d = mapping_[c];
    for (int j = (onesRank*max_length_level + zerosRank*(max_length_level-1));  ; ++j) {
        if ((*leaf_string_)[j] == d) --k;
        if (!k) return s + j - (onesRank*max_length_level + zerosRank*(max_length_level-1));
    }

    return -1;
}

int CBlockTree::better_rank(int c, int i) {

    auto& cumulated_ranks = bt_cumulated_ranks_[c];
    auto& first_cumulated_ranks = bt_first_level_cumulated_ranks_[c];

    auto& second_ranks = bt_second_ranks_[c];
    //auto first_ranks = bt_first_ranks_[c];

    int current_block = i/max_length_first_level_;
    int onesRank = (*(bt_bv_block_length_rank_[0]))(current_block+1);
    int zerosRank = current_block+1 - onesRank;
    int acc = i;
    int delta_ones = onesRank;
    int new_guess = current_block;
    while (onesRank*max_length_first_level_ + zerosRank*(max_length_first_level_ - 1) - 1 < i) {
        acc -= (new_guess+1)*max_length_first_level_ - (new_guess + 1 - delta_ones);
        new_guess = acc/max_length_first_level_;
        current_block += 1 + new_guess;
        delta_ones = -onesRank;
        onesRank = (*(bt_bv_block_length_rank_[0]))(current_block+1);
        delta_ones += onesRank;
        zerosRank = current_block+1 - onesRank;
    }
/*
    if  (j < i) {
        initial_block += (i-j-1)/max_length_first_level_;
        onesRank = (*(bt_bv_block_length_rank_[0]))(initial_block+1);
        zerosRank = initial_block+1 - onesRank;
        j = onesRank*max_length_first_level_ + zerosRank*(max_length_first_level_ - 1) - 1;
    }


    while (j < i) {
        ++initial_block;
        j += ((*(bt_bv_block_length_[0]))[initial_block]) ? max_length_first_level_ : max_length_first_level_ - 1;
    }
*/
    int current_length = ((*(bt_bv_block_length_[0]))[current_block]) ? max_length_first_level_ : max_length_first_level_ - 1;

    i -= onesRank*max_length_first_level_ + zerosRank*(max_length_first_level_ - 1) - current_length;

    int max_length_level = max_length_first_level_;
    int level = 0;

    int r = (current_block == 0) ? 0 : (*first_cumulated_ranks)[current_block-1];

    while (level < number_of_levels_-1) {
        if ((*bt_bv_[level])[current_block]) { // Case InternalBlock
            int big_blocks = (current_length%r_ == 0) ? r_ : current_length%r_;
            current_length = (current_length%r_ == 0) ? current_length/r_ : current_length/r_ + 1;
            if (i < current_length*big_blocks) { //why the difference with alternative? both do this code
                int child_number = i/current_length;
                i -= child_number*current_length;
                current_block = (*bt_bv_rank_[level])(current_block)*r_ + child_number;
            } else {
                i -= current_length*big_blocks;
                --current_length;
                int child_number = big_blocks + i/current_length;
                i -= (child_number-big_blocks) * current_length;
                current_block = (*bt_bv_rank_[level])(current_block)*r_ + child_number;
            }
            ++level;
            r += (current_block%r_ == 0) ? 0 : (*cumulated_ranks[level-1])[current_block-1];
            max_length_level = (max_length_level % r_) ? max_length_level/r_ +1 : max_length_level/r_;
        } else { // Case BackBlock
            int index = current_block - (*bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*bt_offsets_[level])[index];
            current_block = encoded_offset/max_length_level;
            current_length = ((*bt_bv_block_length_[level])[current_block]) ? max_length_level : max_length_level-1;
            int offset = encoded_offset%max_length_level;
            if (!offset) {
                if (i >= current_length) {
                    r += (*second_ranks[level])[index];

                    ++current_block;
                    i -= current_length;
                    current_length = ((*bt_bv_block_length_[level])[current_block]) ? max_length_level :
                                     max_length_level - 1;

                }
            } else {
                i += offset;
                r += (*second_ranks[level])[index];
                if (i >= current_length) {
                    ++current_block;
                    i -= current_length;
                    current_length = ((*bt_bv_block_length_[level])[current_block]) ? max_length_level :
                                     max_length_level - 1;
                } else {
                    if (level != 0 )r -= ((current_block%r_ == 0) ?  ((*cumulated_ranks[level-1])[current_block]) :  (((*cumulated_ranks[level-1])[current_block]) - ((*cumulated_ranks[level-1])[current_block-1])));
                    else r -= ((current_block == 0) ?  ((*first_cumulated_ranks)[current_block]) :  (((*first_cumulated_ranks)[current_block]) - ((*first_cumulated_ranks)[current_block-1])));
                }
            }
        }
    }

    onesRank =  (*bt_bv_block_length_rank_[level])(current_block);
    zerosRank = current_block - onesRank;

    i  += (onesRank*max_length_level + zerosRank*(max_length_level-1));
    int d = mapping_[c];
    for (int j = (onesRank*max_length_level + zerosRank*(max_length_level-1)); j <= i; ++j) {
        if ((*leaf_string_)[j] == d) ++r;
    }

    return r;
    /*
   int qq = (onesRank*max_length_level + zerosRank*(max_length_level-1));
   int rr = (onesRank*max_length_level + zerosRank*(max_length_level-1)) + i;

   return r + leaf_string_wt_->rank(rr+1, c) - leaf_string_wt_->rank(qq, c);
    */
}

int CBlockTree::better_select(int c, int k) {

    auto& cumulated_ranks = bt_cumulated_ranks_[c];
    auto& second_ranks = bt_second_ranks_[c];
    //auto first_ranks = bt_first_ranks_[c];
    auto& first_level_cumulated_ranks = bt_first_level_cumulated_ranks_[c];

    int current_block = (k-1)/max_length_first_level_;

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

    int onesRank = (*(bt_bv_block_length_rank_[0]))(current_block);
    int zerosRank = current_block - onesRank;
    int s = onesRank*max_length_first_level_ + zerosRank*(max_length_first_level_ - 1);
    k -= (current_block == 0) ? 0 : (*first_level_cumulated_ranks)[current_block-1];


    int current_length = ((*(bt_bv_block_length_[0]))[current_block]) ? max_length_first_level_ : max_length_first_level_ - 1;
    int max_length_level = max_length_first_level_;
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
            while ( child < firstChild + r_-1 && k > (*cumulated_ranks[level+1-1])[child]) {
                ++child;
            }

            k -= (child%r_ == 0) ? 0 : (*cumulated_ranks[level+1-1])[child-1];
            int big_blocks = (current_length%r_ == 0) ? r_ : current_length%r_;
            current_length = (current_length%r_ == 0) ? current_length/r_ : current_length/r_ + 1;
            current_block = child;
            if (child-firstChild < big_blocks) {
                s += (child-firstChild)*current_length;
            } else {
                s += big_blocks*current_length + ((child-firstChild)-big_blocks) * (current_length-1);
                --current_length;
            }
            ++level;
            max_length_level = (max_length_level % r_) ? max_length_level/r_ +1 : max_length_level/r_;
        } else { // Case BackBlock
            int index = current_block -  (*bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*bt_offsets_[level])[index];
            current_block = encoded_offset/max_length_level;
            current_length = ((*bt_bv_block_length_[level])[current_block]) ? max_length_level : max_length_level-1;

            int offset = encoded_offset%max_length_level;
            int second_rank = (*second_ranks[level])[index];
            if (!offset) {
                if (k > second_rank) {
                    s += current_length;
                    k -= second_rank;
                    ++current_block;
                    current_length = ((*bt_bv_block_length_[level])[current_block]) ? max_length_level :
                                     max_length_level - 1;
                }
            } else {
                s -= offset;
                k -= second_rank;
                if (k > 0) {
                    s += current_length;
                    ++current_block;
                    current_length = ((*bt_bv_block_length_[level])[current_block]) ? max_length_level :
                                     max_length_level - 1;
                } else {

                    if (level != 0) k += ((current_block%r_ == 0) ?  ((*cumulated_ranks[level-1])[current_block]) :  (((*cumulated_ranks[level-1])[current_block]) - ((*cumulated_ranks[level-1])[current_block-1])) );
                    else k += ((current_block == 0) ?  ((*first_level_cumulated_ranks)[current_block]) :  (((*first_level_cumulated_ranks)[current_block]) - ((*first_level_cumulated_ranks)[current_block-1])));
                }
            }
        }
    }

    onesRank =  (*bt_bv_block_length_rank_[level])(current_block);
    zerosRank = current_block - onesRank;

    int d = mapping_[c];
    for (int j = (onesRank*max_length_level + zerosRank*(max_length_level-1));  ; ++j) {
        if ((*leaf_string_)[j] == d) --k;
        if (!k) return s + j - (onesRank*max_length_level + zerosRank*(max_length_level-1));
    }
/*
    int qq = (onesRank*max_length_level + zerosRank*(max_length_level-1));
    return s+leaf_string_wt_->select(k + leaf_string_wt_->rank(qq, c), c)-qq;
*/
    return -1;
}


void CBlockTree::print_statistics() {
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


    int bt_bv_block_length_size = sizeof(void*);
    for (sdsl::bit_vector* bv : bt_bv_block_length_) {
        bt_bv_block_length_size += sdsl::size_in_bytes(*bv);
    }
    int c_bt_bv_block_length_size = sizeof(void*);
    for (sdsl::rrr_vector<63>* bv : c_bt_bv_block_length_) {
        c_bt_bv_block_length_size += sdsl::size_in_bytes(*bv);
    }


    int bt_bv_block_length_rank_size = sizeof(void*);
    for (sdsl::rank_support_v<1>* bvr : bt_bv_block_length_rank_) {
        bt_bv_block_length_rank_size += sdsl::size_in_bytes(*bvr);
    }
    int c_bt_bv_block_length_rank_size = sizeof(void*);
    for (sdsl::rrr_vector<63>::rank_1_type* bvr : c_bt_bv_block_length_rank_) {
        c_bt_bv_block_length_rank_size += sdsl::size_in_bytes(*bvr);
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
        c_bt_prefix_ranks_first_level_size += sdsl::size_in_bytes(*(c_bt_first_level_prefix_ranks_[pair.first]));
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


    int partial_total_size = mapping_size + alphabet_size +bt_bv_size+ bt_bv_rank_size+ bt_bv_block_length_size + bt_bv_block_length_rank_size + bt_offsets_size + leaf_string_size;
    int c_partial_total_size = mapping_size + alphabet_size +c_bt_bv_size+ c_bt_bv_rank_size+ c_bt_bv_block_length_size + c_bt_bv_block_length_rank_size + c_bt_offsets_size + c_leaf_string_size;

    std::cout << "Bit Vectors Size(B): " << bt_bv_size << std::endl;
    std::cout << "Bit Vectors Rank Size(B): " << bt_bv_rank_size << std::endl;
    std::cout << "Bit Vectors Type Size(B): " << bt_bv_block_length_size << std::endl;
    std::cout << "Bit Vectors Type Rank Size(B): " << bt_bv_block_length_rank_size << std::endl;
    std::cout << "Offsets Size Plain(B): " << bt_offsets_size << std::endl;
    std::cout << "Leaves String Size(B): " << leaf_string_size << std::endl;
    std::cout << "Mapping Size(B): " << mapping_size << std::endl;
    std::cout << "Alphabet Size(B): " << alphabet_size << std::endl;
    std::cout << "Leaves Bitvector Size(B): " << leaf_bv_size << std::endl;
    std::cout << "Leaves Bitvector Rank Size(B): " << leaf_bv_rank_size << std::endl;
    std::cout << "Partial Total Size(B): " << partial_total_size << std::endl<< std::endl;


    std::cout << "Compressed Bit Vectors Size(B): " << c_bt_bv_size << std::endl;
    std::cout << "Compressed Bit Vectors Rank Size(B): " << c_bt_bv_rank_size << std::endl;
    std::cout << "Compressed Bit Vectors Type Size(B): " << c_bt_bv_block_length_size << std::endl;
    std::cout << "Compressed Bit Vectors Type Rank Size(B): " << c_bt_bv_block_length_rank_size << std::endl;
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
    std::cout << "ranks Size Plain(B): " << bt_ranks_total_size << std::endl;
    std::cout << "prefix_ranks Size Plain(B): " << bt_prefix_ranks_total_size << std::endl;
    std::cout << "cumulated_ranks Size Plain(B): " << bt_cumulated_ranks_total_size << std::endl;
    std::cout << "first_ranks Size Plain(B): " << bt_first_ranks_total_size << std::endl;
    std::cout << "second_ranks Size Plain(B): " << bt_second_ranks_total_size << std::endl << std::endl;
    std::cout << "Last Level ranks Size Plain(B): " << bt_last_level_ranks_total_size << std::endl;
    std::cout << "Last Level prefix_ranks Size Plain(B): " << bt_last_level_prefix_ranks_total_size << std::endl;
    std::cout << "Last Level cumulated_ranks Size Plain(B): " << bt_last_level_cumulated_ranks_total_size << std::endl;

    std::cout << std::endl << "Compressed RANK" << std::endl;
    std::cout << "Compressed first_level_prefix_ranks(B): " << c_bt_prefix_ranks_first_level_size << std::endl;
    std::cout << "Compressed first_level_cumulated_ranks(B): " << c_bt_cumulated_ranks_first_level_size << std::endl;
    std::cout << "Compressed ranks Size(B): " << c_bt_ranks_total_size << std::endl;
    std::cout << "Compressed prefix_ranks Size(B): " << c_bt_prefix_ranks_total_size << std::endl;
    std::cout << "Compressed cumulated_ranks Size(B): " << c_bt_cumulated_ranks_total_size << std::endl;
    std::cout << "Compressed first_ranks Size(B): " << c_bt_first_ranks_total_size << std::endl;
    std::cout << "Compressed second_ranks Size(B): " << c_bt_second_ranks_total_size << std::endl << std::endl;
    std::cout << "Compressed Last Level ranks Size(B): " << c_bt_last_level_ranks_total_size << std::endl;
    std::cout << "Compressed Last Level prefix_ranks Size(B): " << c_bt_last_level_prefix_ranks_total_size << std::endl;
    std::cout << "Compressed Last Level cumulated_ranks Size(B): " << c_bt_last_level_cumulated_ranks_total_size << std::endl;


    int rank_paper_version_size = bt_prefix_ranks_first_level_size + bt_first_ranks_total_size + bt_second_ranks_total_size + bt_prefix_ranks_total_size;
    int rank_alternative_version_size = bt_second_ranks_total_size + bt_ranks_total_size + bt_prefix_ranks_first_level_size;
    int rank_better_version_size = bt_second_ranks_total_size + bt_cumulated_ranks_first_level_size + bt_cumulated_ranks_total_size;
    std::cout << "RANK Paper Version Size(B): " << rank_paper_version_size << std::endl;
    std::cout << "RANK Alternative Version Size(B): " << rank_alternative_version_size  << std::endl;
    std::cout << "RANK Better Version Size(B): " << rank_better_version_size  << std::endl << std::endl;

    int c_rank_paper_version_size = c_bt_prefix_ranks_first_level_size + c_bt_first_ranks_total_size + c_bt_second_ranks_total_size + c_bt_prefix_ranks_total_size;
    int c_rank_alternative_version_size = c_bt_second_ranks_total_size + c_bt_ranks_total_size + c_bt_prefix_ranks_first_level_size;
    int c_rank_better_version_size = c_bt_second_ranks_total_size + c_bt_cumulated_ranks_first_level_size + c_bt_cumulated_ranks_total_size;
    std::cout << "Compressed RANK Paper Version Size(B): " << c_rank_paper_version_size << std::endl;
    std::cout << "Compressed RANK Alternative Version Size(B): " << c_rank_alternative_version_size  << std::endl;
    std::cout << "Compressed RANK Better Version Size(B): " << c_rank_better_version_size  << std::endl << std::endl;


    std::cout << "TOTAL" << std::endl;
    std::cout << "Total Size (with paper rank)(B): " << rank_paper_version_size + partial_total_size << std::endl;
    std::cout << "Total Size (with alternative rank)(B): " << rank_alternative_version_size + partial_total_size << std::endl;
    std::cout << "Total Size (with better rank)(B): " << rank_better_version_size + partial_total_size << std::endl << std::endl;


    std::cout << "Compressed WITHOUT RANK COMPRESSION" << std::endl;
    std::cout << "Compressed Total Size (with paper rank)(B): " << rank_paper_version_size + c_partial_total_size << std::endl;
    std::cout << "Compressed Total Size (with alternative rank)(B): " << rank_alternative_version_size + c_partial_total_size << std::endl;
    std::cout << "Compressed Total Size (with better rank)(B): " << rank_better_version_size + c_partial_total_size << std::endl << std::endl;

    std::cout << "Compressed TOTAL" << std::endl;
    std::cout << "Compressed Total Size (with paper rank)(B): " << c_rank_paper_version_size + c_partial_total_size << std::endl;
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

