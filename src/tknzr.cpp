#include "tknzr/tknzr.hpp"
#include <iostream>
#include <array>

namespace tknzr {

// base 64 lookup table
const std::string base64_lt =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_decode(std::string_view encoded_string) {
    int str_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    std::array<unsigned char, 4> char_array_4;
    std::array<unsigned char, 3> char_array_3;
    std::string ret;
    
    while(str_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                char_array_4[i] = base64_lt.find(char_array_4[i]);
            }
            
            // mapping 4 6-bit values to 3 bytes
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            
            for (i = 0; i < 3; i++)
                ret += char_array_3[i];
            i = 0;
        }
    }
    
    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;
        
        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_lt.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
        
        for (j = 0; j < i - 1; j++)
            ret += char_array_3[j];
    }
    
    return ret;
}

Tokenizer::Tokenizer(const std::string& encoder_file);
    
} // namespace tknzr


namespace tknzr {
    
    void tokenize(const std::string& bytestream) {
    }

    size_t PairHash::operator()(const Pair& p) const noexcept {
        return (std::hash<int>()(p.first) << 1) ^ std::hash<int>()(p.second);
    }

    std::unordered_map<Pair, int, PairHash> create_pairs(const std::string& bytestream) {
        std::unordered_map<Pair, int, PairHash> pairs;
        for(int i = 0; i + 1 < bytestream.length(); ++i) {
            pairs[{bytestream[i], bytestream[i+1]}]++;
        }
    
        // For Debugging
        for (auto& [key, val] : pairs) {
         //   std::cout << "Key: (" << key.first << ", " << key.second << ") ->" << ", Val: " << val << std::endl;
        }
    
        return pairs;
    }

    std::unordered_map<Pair, int, PairHash> create_pairs(const std::vector<int>& bytestream) {
        std::unordered_map<Pair, int, PairHash> pairs;
        for(int i = 0; i + 1 < bytestream.size(); ++i) {
            pairs[{bytestream.at(i), bytestream.at(i+1)}]++;
        }
    
        // For Debugging
        for(auto& [key, val] : pairs) {
          //  std::cout << "Key: (" << key.first << ", " << key.second << ") ->" << ", Val: " << val << std::endl;
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

    Pair get_most_common_pair(const std::vector<int>& bytestream) {
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


    // this is a function that works only on the initial bytestream with characters raging in 0-255
    std::vector<int> convert_bytestream_to_vector(const std::string& bytestream) {
        std::vector<int> vec;
        vec.reserve(bytestream.size());

        for(int x : bytestream) {
            vec.emplace_back(x);
        }
    
        return vec;
    }

    // can be optimized for SURE, better algorithm and design etc.
    // This function is changing a pair with a single value
    std::vector<int> swap_pairs_with_value(const std::vector<int>& data, const Pair& pair, int new_key) {
        std::vector<int> new_vector;
        new_vector.reserve(data.size()); // kinda wastefull, but less copies

        int i = 0;
        while(i < data.size()) {
            if (i < data.size() - 1) { // in bounds
                if(data.at(i) == pair.first && data.at(i + 1) == pair.second) {
                    new_vector.emplace_back(new_key);
                    i+=2;
                }
            }
            else {
                new_vector.emplace_back(data.at(i));
                i++;
            }
        }

        return new_vector;
    }

    // returns merges list
    std::unordered_map<int, Pair> create_vocab_n_sized(const std::vector<int>& data, int n) {
        
        std::unordered_map<int, Pair> vocab;
        
        std::vector<int> buff = data;

        for(int i = 0; i < n - 256; ++i) {
            Pair mcp = get_most_common_pair(data);
            
            buff = swap_pairs_with_value(buff, mcp, 256 + i);
            vocab[256 + i] = mcp;
        }
        return vocab;
    }
}

/*
    There is a problem because pair values might not be fitting char,
    because of BPE? Not sure tho, we will see later...
*/
std::ostream& operator<<(std::ostream& os, const tknzr::Pair& pair) {
    return os << 
    "(" << 
    static_cast<char>(pair.first) << 
    ", " << 
    static_cast<char>(pair.second) << ")";
}