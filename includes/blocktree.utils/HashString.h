//
// Created by sking32 on 3/14/18.
//

#ifndef BLOCKTREE_HASHSTRING_H
#define BLOCKTREE_HASHSTRING_H

#include <string>

class HashString {
public:
    std::string aux;
    std::basic_string<int64_t> waux;
    size_t hash_;
    std::string& s_;
    std::basic_string<int64_t>& ws_;
    int init_;
    int end_;

    HashString(size_t, std::string&, int, int);
    HashString(size_t, std::basic_string<int64_t>&, int, int);
    ~HashString();

    bool operator==(const HashString&) const;
};

namespace std {
    template <> struct hash<HashString> {
        std::size_t operator()(const HashString& hS) const {
            return hS.hash_;
        }
    };
}
#endif //BLOCKTREE_HASHSTRING_H
