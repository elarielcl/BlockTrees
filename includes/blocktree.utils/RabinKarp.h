//
// Created by sking32 on 3/12/18.
//

#ifndef BLOCKTREE_RABINKARP_H
#define BLOCKTREE_RABINKARP_H

#include <string>

class RabinKarp {
    uint64_t kp_;
    uint64_t init_;
    uint64_t rm_;

public:
    uint64_t sigma_;
    uint64_t hash_;
    uint64_t size_;
    std::string& s_;
    std::basic_string<int64_t>& ws_;
    std::string aux;
    std::basic_string<int64_t> waux;


    RabinKarp(std::string& s, int init, int size, int range, int sigma = 257);
    RabinKarp(std::basic_string<int64_t>& s, int init, int size, int range, int sigma = 257);

    uint64_t  hash();
    void next();
    void wnext();
};

#endif //BLOCKTREE_RABINKARP_H
