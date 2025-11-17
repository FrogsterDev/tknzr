#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

namespace tknzr {

    using Token = int;
    using Pair = std::pair<Token, Token>;
    using TokenList = std::vector<Token>;

    struct PairHash {
        size_t operator()(const Pair& p) const noexcept; 
    };

    /**
     * Main tokenizer class compatible with GPT API tokenization
     * Uses Byte Pair Encoding (BPE) algorithm
     */
    class Tokenizer {
    public:
        /**
         * Create a tokenizer with a custom vocabulary size
         * @param vocab_size Target vocabulary size (default 100256 for GPT-4/cl100k_base)
         */
        explicit Tokenizer(int vocab_size = 100256);

        /**
         * Load tokenizer from tiktoken format file
         * @param filepath Path to the tokenizer file (base64 encoded merges or binary)
         * @return true if loaded successfully, false otherwise
         */
        bool load_from_file(const std::string& filepath);

        /**
         * Load tokenizer from base64 encoded merges string (tiktoken format)
         * @param base64_merges Base64 encoded merge rules
         * @return true if loaded successfully, false otherwise
         */
        bool load_from_base64(const std::string& base64_merges);

        /**
         * Load tokenizer from tiktoken binary format (GPT-4 compatible)
         * @param binary_data Binary data containing merge rules (little-endian uint16_t pairs)
         * @return true if loaded successfully, false otherwise
         */
        bool load_from_tiktoken_binary(const std::vector<uint8_t>& binary_data);

        /**
         * Train tokenizer on text data
         * @param text Training text
         * @param vocab_size Target vocabulary size
         */
        void train(const std::string& text, int vocab_size);

        /**
         * Encode text into tokens
         * @param text Input text to encode
         * @return Vector of token IDs
         */
        TokenList encode(const std::string& text) const;

        /**
         * Decode tokens back to text
         * @param tokens Vector of token IDs
         * @return Decoded text string
         */
        std::string decode(const TokenList& tokens) const;

        /**
         * Get vocabulary size
         * @return Number of tokens in vocabulary
         */
        size_t vocab_size() const;

        /**
         * Get the merge rules (vocabulary)
         * @return Map from token ID to pair of tokens it represents
         */
        const std::unordered_map<Token, Pair>& get_merges() const;

    private:
        std::unordered_map<Token, Pair> merges_;  // token_id -> (token1, token2)
        std::unordered_map<Pair, Token, PairHash> merge_ranks_;  // (token1, token2) -> rank
        int vocab_size_;
        
        // Helper functions
        std::vector<int> bytes_to_unicode() const;
        std::vector<int> text_to_bytes(const std::string& text) const;
        std::string bytes_to_text(const std::vector<int>& bytes) const;
        std::vector<Token> bpe_encode(const std::vector<int>& word_bytes) const;
        std::vector<int> bpe_decode(const std::vector<Token>& tokens) const;
        std::vector<Pair> get_word_pairs(const std::vector<int>& word) const;
        Pair get_most_common_pair(const std::vector<int>& word) const;
        std::vector<int> apply_merge(const std::vector<int>& word, const Pair& pair, Token new_token) const;
    };

    // Legacy functions (kept for backward compatibility, but deprecated)
    void tokenize(const std::string& bytestream);
    std::unordered_map<Pair, int, PairHash> create_pairs(const std::string& bytestream);
    std::unordered_map<Pair, int, PairHash> create_pairs(const std::vector<int>& bytestream);
    Pair get_most_common_pair(const std::string& bytestream);
    Pair get_most_common_pair(const std::vector<int>& bytestream);
    std::vector<int> convert_bytestream_to_vector(const std::string& bytestream);
    std::vector<int> swap_pairs_with_value(const std::vector<int>& data, const Pair& pair, int new_key);
    std::unordered_map<int, Pair> create_vocab_n_sized(const std::vector<int>& data, int n);
}

std::ostream& operator<<(std::ostream& os, const tknzr::Pair& pair);