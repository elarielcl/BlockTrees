//
// Created by sking32 on 4/10/18.
//

#ifndef BLOCKTREE_NONCONSECUTIVEHASHSTRING_H
#define BLOCKTREE_NONCONSECUTIVEHASHSTRING_H

#include <string>
#include <queue>

class NonConsecutiveHashString {
public:
    size_t hash_;
    int first_block_start_index_;
    int first_block_length_;
    int second_block_start_index_;
    int offset_;
    int size_;
    std::string aux;
    std::basic_string<int64_t> waux;
    std::string& source_;
    std::basic_string<int64_t>& wsource_;

    NonConsecutiveHashString(size_t, int, int, int, std::string&, int, int);
    NonConsecutiveHashString(size_t, int, int, int, std::basic_string<int64_t>&, int, int);
    ~NonConsecutiveHashString();

    bool operator==(const NonConsecutiveHashString&) const;
};

namespace std {
    template <> struct hash<NonConsecutiveHashString> {
        std::size_t operator()(const NonConsecutiveHashString& hS) const {
            return hS.hash_;
        }
    };
}

#endif //BLOCKTREE_NONCONSECUTIVEHASHSTRING_H
