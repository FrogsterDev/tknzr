#include "tknzr/tknzr.hpp"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "=== tknzr - GPT-Compatible Tokenizer Demo ===\n\n";
    
    // Create a tokenizer
    tknzr::Tokenizer tokenizer;
    
    // Train on some text
    std::string training_text = "The quick brown fox jumps over the lazy dog. "
                                "The dog was lazy and the fox was quick. "
                                "Hello world! Hello world! Hello world!";
    
    std::cout << "Training tokenizer on text...\n";
    std::cout << "Training text: \"" << training_text << "\"\n\n";
    
    tokenizer.train(training_text, 300);
    
    std::cout << "Vocabulary size: " << tokenizer.vocab_size() << "\n";
    std::cout << "Number of merges: " << tokenizer.get_merges().size() << "\n\n";
    
    // Test encoding
    std::string test_text = "Hello world!";
    std::cout << "Encoding: \"" << test_text << "\"\n";
    
    auto tokens = tokenizer.encode(test_text);
    std::cout << "Tokens (" << tokens.size() << "): [";
    for (size_t i = 0; i < tokens.size(); ++i) {
        std::cout << tokens[i];
        if (i < tokens.size() - 1) std::cout << ", ";
    }
    std::cout << "]\n\n";
    
    // Test decoding
    std::cout << "Decoding tokens back to text...\n";
    std::string decoded = tokenizer.decode(tokens);
    std::cout << "Decoded: \"" << decoded << "\"\n\n";
    
    // Verify round-trip
    if (decoded == test_text) {
        std::cout << "✓ Round-trip encoding/decoding successful!\n";
    } else {
        std::cout << "✗ Round-trip failed!\n";
        std::cout << "  Original: \"" << test_text << "\"\n";
        std::cout << "  Decoded:  \"" << decoded << "\"\n";
    }
    
    // Show some merge rules
    std::cout << "\nFirst 5 merge rules:\n";
    int count = 0;
    for (const auto& [token_id, pair] : tokenizer.get_merges()) {
        if (count++ >= 5) break;
        std::cout << "  Token " << std::setw(4) << token_id 
                  << " = (" << std::setw(4) << pair.first 
                  << ", " << std::setw(4) << pair.second << ")\n";
    }
    
    return 0;
}

