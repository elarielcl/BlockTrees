//
// Created by sking32 on 4/4/18.
//

#ifndef BLOCKTREE_NONCONSECUTIVERABINKARP_H
#define BLOCKTREE_NONCONSECUTIVERABINKARP_H

#include <string>
#include <queue>

class NonConsecutiveRabinKarp {
    int kp_;
    int rm_;

    uint64_t wkp_;
    uint64_t wrm_;
    std::queue<int> elements_;
    std::queue<uint64_t> welements_;

public:
    int sigma_;
    int hash_;

    uint64_t wsigma_;
    uint64_t whash_;
    int size_;


    NonConsecutiveRabinKarp(std::string& s, int init, int size, int range, int sigma = 257);
    NonConsecutiveRabinKarp(std::basic_string<int64_t>& s, int init, int size, int range, int sigma = 257);

    uint64_t hash();
    void put(int c);
    void wput(uint64_t c);

    std::string represented_string();
};


#endif //BLOCKTREE_NONCONSECUTIVERABINKARP_H
