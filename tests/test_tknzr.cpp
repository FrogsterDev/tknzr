#include "tknzr/tknzr.hpp"
#include <iostream>

int main() {

    std::string data = "aaabbcccc";
    tknzr::tokenize(data);
    
    tknzr::create_pairs(data);
    
    tknzr::Pair pair = tknzr::get_most_common_pair(data);
    std::cout << pair;

    return 0;
}
