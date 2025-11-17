#include "tknzr/tknzr.hpp"
#include <iostream>
#include <array>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdint>

namespace tknzr {

// Base64 lookup table
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

// ============================================================================
// Tokenizer Implementation
// ============================================================================

Tokenizer::Tokenizer(int vocab_size) : vocab_size_(vocab_size) {
    // Initialize with base 256 tokens (one for each byte)
    // Default is 100256 for GPT-4/cl100k_base compatibility
}

std::vector<int> Tokenizer::bytes_to_unicode() const {
    // GPT-2 style byte-to-unicode mapping
    // Handles special characters that might cause issues
    std::vector<int> bs;
    for (int i = 0; i < 256; ++i) {
        bs.push_back(i);
    }
    return bs;
}

std::vector<int> Tokenizer::text_to_bytes(const std::string& text) const {
    std::vector<int> bytes;
    bytes.reserve(text.size());
    for (unsigned char c : text) {
        bytes.push_back(static_cast<int>(c));
    }
    return bytes;
}

std::string Tokenizer::bytes_to_text(const std::vector<int>& bytes) const {
    std::string text;
    text.reserve(bytes.size());
    for (int b : bytes) {
        if (b >= 0 && b < 256) {
            text += static_cast<char>(b);
        }
    }
    return text;
}

std::vector<Pair> Tokenizer::get_word_pairs(const std::vector<int>& word) const {
    std::vector<Pair> pairs;
    if (word.size() < 2) return pairs;
    
    pairs.reserve(word.size() - 1);
    for (size_t i = 0; i < word.size() - 1; ++i) {
        pairs.emplace_back(word[i], word[i + 1]);
    }
    return pairs;
}

Pair Tokenizer::get_most_common_pair(const std::vector<int>& word) const {
    if (word.size() < 2) {
        return {0, 0};
    }
    
    std::unordered_map<Pair, int, PairHash> pair_counts;
    for (size_t i = 0; i < word.size() - 1; ++i) {
        pair_counts[{word[i], word[i + 1]}]++;
    }
    
    Pair best_pair = {word[0], word[1]};
    int best_count = 0;
    
    for (const auto& [pair, count] : pair_counts) {
        if (count > best_count) {
            best_pair = pair;
            best_count = count;
        }
    }
    
    return best_pair;
}

std::vector<int> Tokenizer::apply_merge(const std::vector<int>& word, const Pair& pair, Token new_token) const {
    if (word.size() < 2) return word;
    
    std::vector<int> new_word;
    new_word.reserve(word.size());
    
    size_t i = 0;
    while (i < word.size()) {
        if (i < word.size() - 1 && word[i] == pair.first && word[i + 1] == pair.second) {
            new_word.push_back(new_token);
            i += 2;
        } else {
            new_word.push_back(word[i]);
            i++;
        }
    }
    
    return new_word;
}

std::vector<Token> Tokenizer::bpe_encode(const std::vector<int>& word_bytes) const {
    if (word_bytes.empty()) return {};
    if (word_bytes.size() == 1) return {word_bytes[0]};
    if (merge_ranks_.empty()) {
        // No merges learned, return bytes as tokens
        std::vector<Token> tokens;
        tokens.reserve(word_bytes.size());
        for (int b : word_bytes) {
            tokens.push_back(b);
        }
        return tokens;
    }
    
    std::vector<int> word = word_bytes;
    
    // Apply merges in order of rank (lower rank = higher priority)
    std::vector<std::pair<int, Pair>> sorted_merges;
    for (const auto& [pair, rank] : merge_ranks_) {
        sorted_merges.emplace_back(rank, pair);
    }
    std::sort(sorted_merges.begin(), sorted_merges.end());
    
    // Keep merging until no more merges can be applied
    while (word.size() > 1) {
        bool merged = false;
        
        // Find the highest priority merge that exists in the word
        for (const auto& [rank, pair] : sorted_merges) {
            // Check if this pair exists in the word
            bool pair_exists = false;
            for (size_t i = 0; i < word.size() - 1; ++i) {
                if (word[i] == pair.first && word[i + 1] == pair.second) {
                    pair_exists = true;
                    break;
                }
            }
            
            if (pair_exists) {
                // Apply this merge
                Token new_token = 256 + rank;
                word = apply_merge(word, pair, new_token);
                merged = true;
                break; // Start over with the new word
            }
        }
        
        if (!merged) {
            // No more merges can be applied
            break;
        }
    }
    
    std::vector<Token> tokens;
    tokens.reserve(word.size());
    for (int w : word) {
        tokens.push_back(w);
    }
    return tokens;
}

std::vector<int> Tokenizer::bpe_decode(const std::vector<Token>& tokens) const {
    if (tokens.empty()) return {};
    
    std::vector<int> result;
    result.reserve(tokens.size() * 2); // Rough estimate
    
    for (Token token : tokens) {
        if (token < 256) {
            // Base byte token
            result.push_back(token);
        } else {
            // Look up merge rule
            auto it = merges_.find(token);
            if (it != merges_.end()) {
                const Pair& pair = it->second;
                // Recursively decode the pair
                std::vector<Token> pair_tokens = {pair.first, pair.second};
                std::vector<int> decoded_pair = bpe_decode(pair_tokens);
                result.insert(result.end(), decoded_pair.begin(), decoded_pair.end());
            } else {
                // Unknown token, treat as byte (shouldn't happen in valid vocab)
                if (token < 256) {
                    result.push_back(token);
                }
            }
        }
    }
    
    return result;
}

TokenList Tokenizer::encode(const std::string& text) const {
    if (text.empty()) return {};
    
    // Convert text to bytes
    std::vector<int> bytes = text_to_bytes(text);
    
    // Apply BPE encoding
    std::vector<Token> tokens = bpe_encode(bytes);
    
    return tokens;
}

std::string Tokenizer::decode(const TokenList& tokens) const {
    if (tokens.empty()) return "";
    
    // Decode tokens back to bytes
    std::vector<int> bytes = bpe_decode(tokens);
    
    // Convert bytes to text
    return bytes_to_text(bytes);
}

bool Tokenizer::load_from_tiktoken_binary(const std::vector<uint8_t>& binary_data) {
    // tiktoken format: binary data with little-endian uint16_t pairs
    // Each merge is represented as two 16-bit little-endian integers
    
    if (binary_data.size() < 4 || (binary_data.size() % 4 != 0)) {
        return false; // Must be multiple of 4 bytes (2 uint16_t per merge)
    }
    
    merges_.clear();
    merge_ranks_.clear();
    
    Token rank = 0;
    for (size_t i = 0; i + 3 < binary_data.size(); i += 4) {
        // Read two little-endian uint16_t values
        uint16_t token1 = static_cast<uint16_t>(binary_data[i]) | 
                         (static_cast<uint16_t>(binary_data[i + 1]) << 8);
        uint16_t token2 = static_cast<uint16_t>(binary_data[i + 2]) | 
                         (static_cast<uint16_t>(binary_data[i + 3]) << 8);
        
        Token new_token = 256 + rank;
        Pair pair = {static_cast<Token>(token1), static_cast<Token>(token2)};
        
        merges_[new_token] = pair;
        merge_ranks_[pair] = rank;
        rank++;
    }
    
    vocab_size_ = 256 + merges_.size();
    return !merges_.empty();
}

bool Tokenizer::load_from_base64(const std::string& base64_merges) {
    std::string decoded = base64_decode(base64_merges);
    
    // First, try tiktoken binary format (little-endian uint16_t pairs)
    // This is the format used by GPT-4/cl100k_base
    if (decoded.size() >= 4 && decoded.size() % 4 == 0) {
        std::vector<uint8_t> binary_data(decoded.begin(), decoded.end());
        if (load_from_tiktoken_binary(binary_data)) {
            return true;
        }
    }
    
    // Fall back to text format: try parsing as space-separated token pairs
    std::istringstream iss(decoded);
    std::string line;
    Token rank = 0;
    
    merges_.clear();
    merge_ranks_.clear();
    
    while (std::getline(iss, line)) {
        if (line.empty()) continue;
        
        std::istringstream line_stream(line);
        std::string token1_str, token2_str;
        
        if (line_stream >> token1_str >> token2_str) {
            try {
                Token token1 = std::stoi(token1_str);
                Token token2 = std::stoi(token2_str);
                Token new_token = 256 + rank;
                
                merges_[new_token] = {token1, token2};
                merge_ranks_[{token1, token2}] = rank;
                rank++;
            } catch (...) {
                // Skip invalid lines
                continue;
            }
        }
    }
    
    // If text format worked, we're done
    if (!merges_.empty()) {
        vocab_size_ = 256 + merges_.size();
        return true;
    }
    
    // Last resort: try simple byte pairs (for very old formats)
    if (decoded.size() >= 2) {
        rank = 0;
        for (size_t i = 0; i + 1 < decoded.size(); i += 2) {
            Token token1 = static_cast<unsigned char>(decoded[i]);
            Token token2 = static_cast<unsigned char>(decoded[i + 1]);
            Token new_token = 256 + rank;
            
            merges_[new_token] = {token1, token2};
            merge_ranks_[{token1, token2}] = rank;
            rank++;
        }
        vocab_size_ = 256 + merges_.size();
        return !merges_.empty();
    }
    
    return false;
}

bool Tokenizer::load_from_file(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    // Read file content
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> binary_data(file_size);
    file.read(reinterpret_cast<char*>(binary_data.data()), file_size);
    file.close();
    
    // Try loading as tiktoken binary format first (GPT-4 compatible)
    if (file_size >= 4 && file_size % 4 == 0) {
        if (load_from_tiktoken_binary(binary_data)) {
            return true;
        }
    }
    
    // Try as base64 encoded string
    std::string content(binary_data.begin(), binary_data.end());
    return load_from_base64(content);
}

void Tokenizer::train(const std::string& text, int vocab_size) {
    vocab_size_ = vocab_size;
    merges_.clear();
    merge_ranks_.clear();
    
    // Convert text to bytes
    std::vector<int> data = text_to_bytes(text);
    
    // Build vocabulary using BPE algorithm
    std::vector<int> current_data = data;
    Token next_token = 256;
    
    while (next_token < vocab_size && current_data.size() > 1) {
        // Find most common pair
        Pair mcp = get_most_common_pair(current_data);
        
        if (mcp.first == 0 && mcp.second == 0) {
            break; // No more pairs to merge
        }
        
        // Check if this pair already exists
        if (merge_ranks_.find(mcp) != merge_ranks_.end()) {
            break; // Already merged
        }
        
        // Add merge rule
        merges_[next_token] = mcp;
        merge_ranks_[mcp] = next_token - 256;
        
        // Apply merge to data
        current_data = apply_merge(current_data, mcp, next_token);
        
        next_token++;
    }
    
    vocab_size_ = next_token;
}

size_t Tokenizer::vocab_size() const {
    return vocab_size_;
}

const std::unordered_map<Token, Pair>& Tokenizer::get_merges() const {
    return merges_;
}

// ============================================================================
// Legacy Functions (for backward compatibility)
// ============================================================================

void tokenize(const std::string& bytestream) {
    // Deprecated: use Tokenizer class instead
}

size_t PairHash::operator()(const Pair& p) const noexcept {
    return (std::hash<int>()(p.first) << 1) ^ std::hash<int>()(p.second);
}

std::unordered_map<Pair, int, PairHash> create_pairs(const std::string& bytestream) {
    std::unordered_map<Pair, int, PairHash> pairs;
    for(size_t i = 0; i + 1 < bytestream.length(); ++i) {
        pairs[{static_cast<int>(static_cast<unsigned char>(bytestream[i])), 
               static_cast<int>(static_cast<unsigned char>(bytestream[i+1]))}]++;
    }
    return pairs;
}

std::unordered_map<Pair, int, PairHash> create_pairs(const std::vector<int>& bytestream) {
    std::unordered_map<Pair, int, PairHash> pairs;
    for(size_t i = 0; i + 1 < bytestream.size(); ++i) {
        pairs[{bytestream.at(i), bytestream.at(i+1)}]++;
    }
    return pairs;
}

Pair get_most_common_pair(const std::string& bytestream) {
    std::unordered_map<Pair, int, PairHash> pairs = create_pairs(bytestream);

    Pair best_pair = {0, 0};
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

    Pair best_pair = {0, 0};
    int best_pair_counter = 0;

    for (const auto& [pair, count] : pairs) {
        if (count > best_pair_counter) {
            best_pair = pair;
            best_pair_counter = count;
        }
    }

    return best_pair;
}

std::vector<int> convert_bytestream_to_vector(const std::string& bytestream) {
    std::vector<int> vec;
    vec.reserve(bytestream.size());

    for(unsigned char c : bytestream) {
        vec.emplace_back(static_cast<int>(c));
    }

    return vec;
}

std::vector<int> swap_pairs_with_value(const std::vector<int>& data, const Pair& pair, int new_key) {
    std::vector<int> new_vector;
    new_vector.reserve(data.size());

    size_t i = 0;
    while(i < data.size()) {
        if (i < data.size() - 1) {
            if(data.at(i) == pair.first && data.at(i + 1) == pair.second) {
                new_vector.emplace_back(new_key);
                i+=2;
            } else {
                new_vector.emplace_back(data.at(i));
                i++;
            }
        } else {
            new_vector.emplace_back(data.at(i));
            i++;
        }
    }

    return new_vector;
}

// Fixed bug: was using 'data' instead of 'buff'
std::unordered_map<int, Pair> create_vocab_n_sized(const std::vector<int>& data, int n) {
    std::unordered_map<int, Pair> vocab;
    
    std::vector<int> buff = data;

    for(int i = 0; i < n - 256 && buff.size() > 1; ++i) {
        Pair mcp = get_most_common_pair(buff);  // Fixed: use buff instead of data
        
        if (mcp.first == 0 && mcp.second == 0) {
            break; // No more pairs
        }
        
        buff = swap_pairs_with_value(buff, mcp, 256 + i);
        vocab[256 + i] = mcp;
    }
    return vocab;
}

} // namespace tknzr

std::ostream& operator<<(std::ostream& os, const tknzr::Pair& pair) {
    return os << "(" << pair.first << ", " << pair.second << ")";
}
