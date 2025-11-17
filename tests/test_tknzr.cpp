#include "tknzr/tknzr.hpp"
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <cstdint>

// Test basic tokenizer creation
TEST(TokenizerTest, BasicCreation) {
    tknzr::Tokenizer tokenizer;
    EXPECT_EQ(tokenizer.vocab_size(), 100256); // Default is GPT-4/cl100k_base size
    
    tknzr::Tokenizer custom_tokenizer(1000);
    EXPECT_EQ(custom_tokenizer.vocab_size(), 1000);
    
    tknzr::Tokenizer gpt2_tokenizer(50257); // GPT-2 size
    EXPECT_EQ(gpt2_tokenizer.vocab_size(), 50257);
}

// Test training on simple text
TEST(TokenizerTest, TrainSimpleText) {
    tknzr::Tokenizer tokenizer;
    std::string text = "hello world";
    tokenizer.train(text, 300);
    
    EXPECT_GT(tokenizer.vocab_size(), 256);
    EXPECT_FALSE(tokenizer.get_merges().empty());
}

// Test encoding and decoding
TEST(TokenizerTest, EncodeDecode) {
    tknzr::Tokenizer tokenizer;
    std::string text = "hello";
    tokenizer.train(text, 300);
    
    auto tokens = tokenizer.encode(text);
    EXPECT_FALSE(tokens.empty());
    
    std::string decoded = tokenizer.decode(tokens);
    EXPECT_EQ(decoded, text);
}

// Test encoding empty string
TEST(TokenizerTest, EncodeEmptyString) {
    tknzr::Tokenizer tokenizer;
    tokenizer.train("test", 300);
    
    auto tokens = tokenizer.encode("");
    EXPECT_TRUE(tokens.empty());
    
    std::string decoded = tokenizer.decode(tokens);
    EXPECT_EQ(decoded, "");
}

// Test decoding empty tokens
TEST(TokenizerTest, DecodeEmptyTokens) {
    tknzr::Tokenizer tokenizer;
    tokenizer.train("test", 300);
    
    tknzr::TokenList empty_tokens;
    std::string decoded = tokenizer.decode(empty_tokens);
    EXPECT_EQ(decoded, "");
}

// Test round-trip encoding/decoding
TEST(TokenizerTest, RoundTrip) {
    tknzr::Tokenizer tokenizer;
    std::string text = "The quick brown fox jumps over the lazy dog";
    tokenizer.train(text, 500);
    
    auto tokens = tokenizer.encode(text);
    std::string decoded = tokenizer.decode(tokens);
    
    EXPECT_EQ(decoded, text);
}

// Test with repeated patterns (should create merges)
TEST(TokenizerTest, RepeatedPatterns) {
    tknzr::Tokenizer tokenizer;
    std::string text = "aaabbcccc";
    tokenizer.train(text, 300);
    
    auto tokens = tokenizer.encode(text);
    EXPECT_FALSE(tokens.empty());
    
    std::string decoded = tokenizer.decode(tokens);
    EXPECT_EQ(decoded, text);
}

// Test with special characters
TEST(TokenizerTest, SpecialCharacters) {
    tknzr::Tokenizer tokenizer;
    std::string text = "Hello, World! 123 @#$%";
    tokenizer.train(text, 400);
    
    auto tokens = tokenizer.encode(text);
    std::string decoded = tokenizer.decode(tokens);
    
    EXPECT_EQ(decoded, text);
}

// Test with unicode characters
TEST(TokenizerTest, UnicodeCharacters) {
    tknzr::Tokenizer tokenizer;
    std::string text = "Hello 世界 🌍";
    tokenizer.train(text, 400);
    
    auto tokens = tokenizer.encode(text);
    std::string decoded = tokenizer.decode(tokens);
    
    EXPECT_EQ(decoded, text);
}

// Test PairHash
TEST(PairHashTest, BasicHash) {
    tknzr::PairHash hasher;
    tknzr::Pair p1 = {1, 2};
    tknzr::Pair p2 = {1, 2};
    tknzr::Pair p3 = {2, 1};
    
    EXPECT_EQ(hasher(p1), hasher(p2));
    EXPECT_NE(hasher(p1), hasher(p3));
}

// Test legacy functions (backward compatibility)
TEST(LegacyFunctionsTest, CreatePairs) {
    std::string text = "hello";
    auto pairs = tknzr::create_pairs(text);
    EXPECT_GT(pairs.size(), 0);
}

TEST(LegacyFunctionsTest, GetMostCommonPair) {
    std::string text = "aaabb";
    auto pair = tknzr::get_most_common_pair(text);
    EXPECT_EQ(pair.first, 'a');
    EXPECT_EQ(pair.second, 'a');
}

TEST(LegacyFunctionsTest, ConvertBytestream) {
    std::string text = "hello";
    auto vec = tknzr::convert_bytestream_to_vector(text);
    EXPECT_EQ(vec.size(), text.size());
    EXPECT_EQ(vec[0], static_cast<int>(static_cast<unsigned char>('h')));
}

TEST(LegacyFunctionsTest, SwapPairs) {
    std::vector<int> data = {1, 2, 1, 2, 3};
    tknzr::Pair pair = {1, 2};
    auto result = tknzr::swap_pairs_with_value(data, pair, 99);
    
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 99);
    EXPECT_EQ(result[1], 99);
    EXPECT_EQ(result[2], 3);
}

TEST(LegacyFunctionsTest, CreateVocabNSized) {
    std::vector<int> data = {1, 2, 1, 2, 1, 2, 3, 4};
    auto vocab = tknzr::create_vocab_n_sized(data, 260);
    
    EXPECT_GT(vocab.size(), 0);
    EXPECT_LE(vocab.size(), 4); // n - 256 = 4
}

// Test that encoding produces fewer tokens for repeated patterns
TEST(TokenizerTest, Compression) {
    tknzr::Tokenizer tokenizer;
    std::string text = "the the the the";
    tokenizer.train(text, 300);
    
    auto tokens = tokenizer.encode(text);
    // Should have fewer tokens than bytes after training
    EXPECT_LE(tokens.size(), text.size());
    
    std::string decoded = tokenizer.decode(tokens);
    EXPECT_EQ(decoded, text);
}

// Test multiple encodings produce consistent results
TEST(TokenizerTest, Consistency) {
    tknzr::Tokenizer tokenizer;
    std::string text = "hello world";
    tokenizer.train(text, 300);
    
    auto tokens1 = tokenizer.encode(text);
    auto tokens2 = tokenizer.encode(text);
    
    EXPECT_EQ(tokens1, tokens2);
}

// Test tiktoken binary format loading (GPT-4 compatible)
TEST(TokenizerTest, TiktokenBinaryFormat) {
    tknzr::Tokenizer tokenizer;
    
    // Create a simple tiktoken binary format: little-endian uint16_t pairs
    // Example: merge token1=256, token2=257 -> new token 256
    std::vector<uint8_t> binary_data;
    
    // First merge: (256, 257) -> token 256
    uint16_t token1 = 256;
    uint16_t token2 = 257;
    binary_data.push_back(token1 & 0xFF);        // LSB
    binary_data.push_back((token1 >> 8) & 0xFF);  // MSB
    binary_data.push_back(token2 & 0xFF);         // LSB
    binary_data.push_back((token2 >> 8) & 0xFF); // MSB
    
    // Second merge: (258, 259) -> token 257
    uint16_t token3 = 258;
    uint16_t token4 = 259;
    binary_data.push_back(token3 & 0xFF);
    binary_data.push_back((token3 >> 8) & 0xFF);
    binary_data.push_back(token4 & 0xFF);
    binary_data.push_back((token4 >> 8) & 0xFF);
    
    EXPECT_TRUE(tokenizer.load_from_tiktoken_binary(binary_data));
    EXPECT_EQ(tokenizer.vocab_size(), 258); // 256 base + 2 merges
    
    const auto& merges = tokenizer.get_merges();
    EXPECT_EQ(merges.size(), 2);
    EXPECT_EQ(merges.at(256), std::make_pair(256, 257));
    EXPECT_EQ(merges.at(257), std::make_pair(258, 259));
}

// Test GPT-4 default vocabulary size
TEST(TokenizerTest, GPT4DefaultVocabSize) {
    tknzr::Tokenizer tokenizer; // Default should be 100256 for GPT-4
    EXPECT_EQ(tokenizer.vocab_size(), 100256);
}
