//
// Created by sking32 on 3/13/18.
//

#ifndef BLOCKTREE_BACKWARDRABINKARP_H
#define BLOCKTREE_BACKWARDRABINKARP_H

#include <string>

class BackwardRabinKarp {
    int kp_;
    int init_;
    int rm_;

public:
    int sigma_;
    int size_;
    int hash_;
    std::string& s_;


    BackwardRabinKarp(std::string& s, int init, int size, int range, int sigma = 2); // Use this for the map and copy constructor

    int hash();
    void next();

};

#endif //BLOCKTREE_BACKWARDRABINKARP_H
