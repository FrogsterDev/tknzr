#include "tknzr/tknzr.hpp"

int main() {

    std::string data = "aaabbcccc";
    tknzr::tokenize(data);
    
    tknzr::create_pairs(data);

    return 0;
}
