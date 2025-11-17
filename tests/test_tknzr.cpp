#include "tknzr/tknzr.hpp"
#include <iostream>

int main() {

    std::string data = "aaabbcccc";
    tknzr::tokenize(data);
    
    tknzr::create_pairs(data);
    
    tknzr::Pair pair = tknzr::get_most_common_pair(data);
    
    std::cout << "pre" << std::endl;

    std::vector<int> t_data = tknzr::convert_bytestream_to_vector(data);
    std::unordered_map<int, tknzr::Pair> vocab = tknzr::create_vocab_n_sized(t_data, 276);
    
    std::cout << "post" << std::endl;
    
    for(auto& [k, v] : vocab) {
        std::cout << k << "-> (" << v.first << ", " << v.second << ")" << std::endl;
    }


    return 0;
}
