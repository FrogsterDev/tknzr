#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <cstddef>

namespace tknzr {

    using Token = int;
    using Pair = std::pair<Token, Token>;

    void tokenize(const std::string& bytestream);

    struct PairHash {
        size_t operator()(const Pair& p) const noexcept; 
    };

    std::unordered_map<Pair, int, PairHash> create_pairs(const std::string& bytestream);
    Pair get_most_common_pair(const std::string& bytestream);
}