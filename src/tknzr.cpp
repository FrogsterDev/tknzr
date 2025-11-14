#include "tknzr/tknzr.hpp"
#include <iostream>


namespace tknzr {
    
    void tokenize(const std::string& bytestream) {
        std::cout << bytestream << std::endl;
    

        
    }

    size_t PairHash::operator()(const Pair& p) const noexcept {
        return (std::hash<int>()(p.first) << 1) ^ std::hash<int>()(p.second);
    }

    std::unordered_map<Pair, int, PairHash> create_pairs(const std::string& bytestream) {
        std::unordered_map<Pair, int, PairHash> pairs;
        for(int i = 0; i + 1 < bytestream.length(); ++i) {
            pairs[{bytestream[i], bytestream[i+1]}]++;
        }
    
        for (auto& [key, val] : pairs) {
            std::cout << "Key: (" << key.first << ", " << key.second << ") ->" << ", Val: " << val << std::endl;
        }
    
        return pairs;
    }

    Pair get_most_common_pair(const std::string& bytestream) {
        std::unordered_map<Pair, int, PairHash> pairs = create_pairs(bytestream);

        Pair best_pair;
        int best_pair_counter = 0;
    
        for (const auto& [pair, count] : pairs) {
            if (count > best_pair_counter) {
                best_pair = pair;
                best_pair_counter = count;
            }
        }
    
        return best_pair;
    }
    
}