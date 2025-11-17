#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <cstddef>
#include <cstdint>

namespace tknzr {
    
class Tokenizer {
public:
    using Rank = uint32_t;
    using ByteString = std::string;
    
    Tokenizer(const std::string& encoder_file);
    ~Tokenizer();

};

} // namespace tknzr


// Custom hashing function for std::vector<unsigned int>
namespace tknzr {

    using Token = int;
    using Pair = std::pair<Token, Token>;

    void tokenize(const std::string& bytestream);

    struct PairHash {
        size_t operator()(const Pair& p) const noexcept; 
    };

    // should be private
    std::unordered_map<Pair, int, PairHash> create_pairs(const std::string& bytestream);
    std::unordered_map<Pair, int, PairHash> create_pairs(const std::vector<int>& bytestream);

    Pair get_most_common_pair(const std::string& bytestream);
    Pair get_most_common_pair(const std::vector<int>& bytestream);

    std::vector<int> convert_bytestream_to_vector(const std::string& bytestream);
    std::vector<int> swap_pairs_with_value(const std::vector<int>& data, const Pair& pair, int new_key);
    std::unordered_map<int, Pair> create_vocab_n_sized(const std::vector<int>& data, int n);
}

std::ostream& operator<<(std::ostream& os, const tknzr::Pair& pair);