//
// Created by sking32 on 5/18/18.
//

#include <iostream>

#include "padding_blocktree/PLeafBlock.h"

PLeafBlock::PLeafBlock(PBlock* parent, int64_t start_index, int64_t end_index, int r, int leaf_length, std::string& source, int child_number):
        PBlock(parent, start_index, end_index, r, leaf_length, source, child_number), data_(source.substr(start_index, end_index-start_index+1)) {
}

PLeafBlock::PLeafBlock(PBlock* parent, int64_t start_index, int64_t end_index, int r, int leaf_length, std::basic_string<int64_t>& source, int child_number):
        PBlock(parent, start_index, end_index, r, leaf_length, source, child_number), wdata_(source.substr(start_index, end_index-start_index+1)) {
}

PLeafBlock::~PLeafBlock() {

}

int PLeafBlock::add_rank_select_support(int c) {
    ranks_[c] = rank(c, (length() < data_.size()) ? length()-1 : data_.size()-1);
    return ranks_[c];
}

int64_t PLeafBlock::add_differential_access_support() {
    sum_ = differential_access((length() < wdata_.size()) ? length()-1: wdata_.size()-1);
    return sum_;
}

int PLeafBlock::test_rank(int c, int i, int& counter) {
    ++counter;
    int r = 0;
    for (int j = 0; j<=i; ++j) {
        if (data_[j] == c) ++r;
    }
    return r;
}

int64_t PLeafBlock::differential_access(int i) {
    int64_t r = 0;
    for (int j = 0; j <= i; ++j) {
        r += wdata_[j];
    }
    return r;
}

int PLeafBlock::rank(int c, int i) {
    int r = 0;
    for (int j = 0; j<=i; ++j) {
        if (data_[j] == c) ++r;
    }
    return r;
}

int64_t PLeafBlock::differential_access_alternative(int i) {
    int64_t r = 0;
    for (int j = 0; j<=i; ++j) {
        r += wdata_[j];
    }
    return r;
}

int PLeafBlock::rank_alternative(int c, int i) {
    int r = 0;
    for (int j = 0; j<=i; ++j) {
        if (data_[j] == c) ++r;
    }
    return r;
}

int PLeafBlock::better_rank(int c, int i) {
    int r = 0;
    for (int j = 0; j<=i; ++j) {
        if (data_[j] == c) ++r;
    }
    return r;
}

int64_t PLeafBlock::better_differential_access(int i) {
    int64_t r = 0;
    for (int j = 0; j<=i; ++j) {
        r += wdata_[j];
    }
    return r;
}

int PLeafBlock::select(int c, int j) {
    for (int i = 0; i < data_.size(); ++i) {
        if (((int)(data_[i])) == c) --j;
        if (!j) return i;
    }
    return -1;
}


int PLeafBlock::select_alternative(int c, int j) {
    for (int i = 0; i < data_.size(); ++i) {
        if (((int)(data_[i])) == c) --j;
        if (!j) return i;
    }
    return -1;
}

int PLeafBlock::better_select(int c, int j) {
    for (int i = 0; i < data_.size(); ++i) {
        if (((int)(data_[i])) == c) --j;
        if (!j) return i;
    }
    return -1;
}

void PLeafBlock::print() {
    std::cout << "\"" << data_ << "\" ";
}

int PLeafBlock::access(int i) {
    return data_[i];
}

int PLeafBlock::waccess(int i) {
    return wdata_[i];
}

int PLeafBlock::access_2(int i, int& a) {
    return data_[i];
}

bool PLeafBlock::check_heuristic() {
    int l = length();
    return l<r_ || l<=leaf_length_;

}

bool PLeafBlock::check() {
    int l = length();
    return l<r_ || l<=leaf_length_;
}

int PLeafBlock::number_of_leaf_blocks() {
    return 1;
}

int PLeafBlock::add_fwdsearch_support() {
    int excess = 0;
    int min_excess = 1;
    std::string rep = represented_string();
    for (char c : rep) {
        excess += (c == source_[0]) ? 1 : -1;
        if (excess < min_excess) min_excess = excess;
    }
    min_prefix_excess_ = min_excess;
    return min_prefix_excess_;
}

int PLeafBlock::add_preffix_min_count_fields() {
    int excess = 0;
    int min_excess = 1;
    int count = 0;
    std::string rep = represented_string();
    for (char c : rep) {
        excess += (c == source_[0]) ? 1 : -1;
        if (excess == min_excess) ++count;
        if (excess < min_excess) {
            min_excess = excess;
            count = 1;
        }
    }
    prefix_min_count_ = count;
    min_prefix_excess_ = min_excess;
    return min_prefix_excess_;
}

int PLeafBlock::add_max_fields() {
    int excess = 0;
    int max_excess = -1;
    std::string rep = represented_string();
    for (char c : rep) {
        excess += (c == source_[0]) ? 1 : -1;
        if (excess > max_excess) max_excess = excess;
    }
    max_prefix_excess_ = max_excess;
    return max_prefix_excess_;
}

int PLeafBlock::fwdsearch(int i, int d, int& e) {
    int a = -1;
    for (int j = i+1; j < data_.size(); ++j) {
        e += (data_[j] == source_[0]) ? 1 : -1;
        if (a == -1  && e == d) a = j;
    }
    return a;
}

int PLeafBlock::positive_fwdsearch(int i, int d, int& e) {
    int a = -1;
    for (int j = i+1; j < data_.size(); ++j) {
        e += (data_[j] == source_[0]) ? 1 : -1;
        if (a == -1  && e == d) a = j;
    }
    return a;
}

int PLeafBlock::bwdsearch(int i, int d, int& e) {
    int a = source_.length();
    for (int j = i; j >= 0; --j) {
        e += (data_[j] == source_[0]) ? 1 : -1;
        if (a == source_.length() && e == -d) a = j-1;
    }
    return a;
}

int PLeafBlock::positive_bwdsearch(int i, int d, int& e) {
    int a = source_.length();
    for (int j = i; j >= 0; --j) {
        e += (data_[j] == source_[0]) ? 1 : -1;
        if (a == source_.length() && e == -d) a = j-1;
    }
    return a;
}

int PLeafBlock::min_excess(int i, int j, int& e) {
    if (i == 0 && (j == length()-1 || j == source_.length() - start_index_ - 1)) {
        for (auto pair : ranks_) {
            if (pair.first == source_[0]) e += pair.second;
            else e -= pair.second;
        }
        return min_prefix_excess_;
    }

    int excess = 0;
    int m = 2;
    for (int k = i; k<=j; ++k) {
        excess += (data_[k] == source_[0]) ? 1 : -1;
        if (excess < m) m = excess;
    }
    e += excess;
    return m;

}

int PLeafBlock::min_select(int i, int j, int &t, int& e, int m) {
    int excess = 0;
    for (int k = i; k<=j; ++k) {
        excess += (data_[k] == source_[0]) ? 1 : -1;
        if (excess == m) --t;
        if (t == 0) return k;
    }
    e += excess;
    return -1;
}

int PLeafBlock::min_count(int i, int j, int& e, int& m) {
    if (i == 0 && (j == length()-1 || j == source_.length() - start_index_ - 1)) {
        for (auto pair : ranks_) {
            if (pair.first == source_[0]) e += pair.second;
            else e -= pair.second;
        }
        m = min_prefix_excess_;
        return prefix_min_count_;
    }

    int excess = 0;
    m = 2;
    int count = 0;
    for (int k = i; k<=j; ++k) {
        excess += (data_[k] == source_[0]) ? 1 : -1;
        if (excess == m) ++count;
        if (excess < m) {
            m = excess;
            count = 1;
        }
    }
    e += excess;
    return count;

}



int PLeafBlock::max_excess(int i, int j, int& e) {
    if (i == 0 && (j == length()-1 || j == source_.length() - start_index_ - 1)) {
        for (auto pair : ranks_) {
            if (pair.first == source_[0]) e += pair.second;
            else e -= pair.second;
        }
        return max_prefix_excess_;
    }

    int excess = 0;
    int M = -1;
    for (int k = i; k<=j; ++k) {
        excess += (data_[k] == source_[0]) ? 1 : -1;
        if (excess > M) M = excess;
    }
    e += excess;
    return M;

}

int PLeafBlock::add_rank_select_leaf_support() {
    starts_with_end_leaf_ = source_[start_index_] != source_[0] && source_[start_index_-1] == source_[0];
    leaf_rank_ = (starts_with_end_leaf_) ? 1 : 0;
    std::string rep = represented_string();
    bool one_seen = false;

    for (char c: rep) {
        if (c == source_[0]) {
            one_seen = true;
        }
        else  {
            if (one_seen) {
                ++leaf_rank_;
            }
            one_seen = false;
        }
    }

    return leaf_rank_;
}

int PLeafBlock::leaf_rank(int i) {
    if (i == -1) return 0;
    int r = 0;
    bool one_seen = starts_with_end_leaf_;
    for (int j = 0; j<=i; ++j) {
        if (data_[j] == source_[0]) {
            one_seen = true;
        } else {
            if (one_seen) {
                ++r;
            }
            one_seen = false;
        }
    }
    return r;
}

int PLeafBlock::leaf_rank_alternative(int i) {
    if (i == -1) return 0;
    int r = 0;
    bool one_seen = starts_with_end_leaf_;
    for (int j = 0; j<=i; ++j) {
        if (data_[j] == source_[0]) {
            one_seen = true;
        } else {
            if (one_seen) {
                ++r;
            }
            one_seen = false;
        }
    }
    return r;
}

int PLeafBlock::better_leaf_rank(int i) {
    if (i == -1) return 0;
    int r = 0;
    bool one_seen = starts_with_end_leaf_;
    for (int j = 0; j<=i; ++j) {
        if (data_[j] == source_[0]) {
            one_seen = true;
        } else {
            if (one_seen) {
                ++r;
            }
            one_seen = false;
        }
    }
    return r;
}


int PLeafBlock::leaf_select(int j) {
    if (starts_with_end_leaf_ && j == 1) return -1;
    bool one_seen = starts_with_end_leaf_;
    for (int i = 0; i < data_.size(); ++i) {
        if (data_[i] == source_[0]) {
            one_seen = true;
        } else {
            if (one_seen) {
                --j;
            }
            one_seen = false;
        }
        if (!j) return i-1;
    }
    return -1;
}


int PLeafBlock::leaf_select_alternative(int j) {
    if (starts_with_end_leaf_ && j == 1) return -1;
    bool one_seen = starts_with_end_leaf_;
    for (int i = 0; i < data_.size(); ++i) {
        if (data_[i] == source_[0]) {
            one_seen = true;
        } else {
            if (one_seen) {
                --j;
            }
            one_seen = false;
        }
        if (!j) return i-1;
    }
    return -1;
}

int PLeafBlock::better_leaf_select(int j) {
    if (starts_with_end_leaf_ && j == 1) return -1;
    bool one_seen = starts_with_end_leaf_;
    for (int i = 0; i < data_.size(); ++i) {
        if (data_[i] == source_[0]) {
            one_seen = true;
        } else {
            if (one_seen) {
                --j;
            }
            one_seen = false;
        }
        if (!j) return i - 1;
    }
    return -1;
}

int PLeafBlock::test_fwdsearch(int i, int d, int& e, int& c) {
    ++c;
    int a = -1;
    for (int j = i+1; j < data_.size(); ++j) {
        e += (data_[j] == source_[0]) ? 1 : -1;
        if (a == -1  && e == d) a = j;
    }
    return a;
}

int PLeafBlock::test_fwdsearch(int i, int k, int d, int& e, int& c) {

    if (i == -1 && (k == length()-1 || k == source_.length() - start_index_ - 1) && e + min_prefix_excess_ > d) {
        for (auto pair : ranks_) {
            if (pair.first == source_[0]) e += pair.second;
            else e -= pair.second;
        }
        return -1;
    }

    ++c;
    for (int j = i+1; j <= k; ++j) {
        e += (data_[j] == source_[0]) ? 1 : -1;
        if (e == d) return j;
    }
    return -1;
}


int PLeafBlock::test_bwdsearch(int i, int d, int& e, int& c) {
    ++c;
    int a = source_.length();
    for (int j = i; j >= 0; --j) {
        e += (data_[j] == source_[0]) ? 1 : -1;
        if (a == source_.length() && e == -d) a = j-1;
    }
    return a;
}

int PLeafBlock::test_bwdsearch(int i, int k, int d, int& e, int& c) {
    ++c;
    int a = source_.length();
    for (int j = k; j >= i; --j) {
        e += (data_[j] == source_[0]) ? 1 : -1;
        if (a == source_.length() && e == -d) a = j-1;
    }
    return a;
}


int PLeafBlock::test_min_excess(int i, int j, int& e, int& c) {

    if (i == 0 && (j == length()-1 || j == source_.length() - start_index_ - 1)) {
        for (auto pair : ranks_) {
            if (pair.first == source_[0]) e += pair.second;
            else e -= pair.second;
        }
        return min_prefix_excess_;
    }

    ++c;

    int excess = 0;
    int m = 2;
    for (int k = i; k<=j; ++k) {
        excess += (data_[k] == source_[0]) ? 1 : -1;
        if (excess < m) m = excess;
    }
    e += excess;
    return m;

}