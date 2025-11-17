# tknzr - GPT-Compatible Tokenizer Library

A C++ library for tokenization compatible with GPT API, implementing Byte Pair Encoding (BPE) algorithm.

## Features

- **GPT-4 Compatible**: Full support for GPT-4's cl100k_base tokenizer via tiktoken format
- **GPT-2 Compatible**: Supports GPT-2 tokenizer format
- **Train Custom Tokenizers**: Train tokenizers on your own text data
- **Load Pre-trained Models**: Load tokenizers from tiktoken format files (binary and base64)
- **Encode/Decode**: Convert text to tokens and back
- **Modern C++20**: Uses modern C++ features for performance and safety

## Building

```bash
mkdir build && cd build
cmake ..
make
```

## Usage

### Basic Usage

```cpp
#include "tknzr/tknzr.hpp"
#include <iostream>

int main() {
    // Create a tokenizer
    tknzr::Tokenizer tokenizer;
    
    // Train on text data
    std::string text = "Hello, world! This is a test.";
    tokenizer.train(text, 500);  // vocab_size = 500
    
    // Encode text to tokens
    auto tokens = tokenizer.encode("Hello, world!");
    std::cout << "Number of tokens: " << tokens.size() << std::endl;
    
    // Decode tokens back to text
    std::string decoded = tokenizer.decode(tokens);
    std::cout << "Decoded: " << decoded << std::endl;
    
    return 0;
}
```

### Loading Pre-trained Tokenizers

#### GPT-4 / tiktoken Format

```cpp
#include "tknzr/tknzr.hpp"
#include <fstream>
#include <vector>

int main() {
    tknzr::Tokenizer tokenizer;
    
    // Load GPT-4 tokenizer from tiktoken binary file
    // The file should contain little-endian uint16_t pairs
    if (tokenizer.load_from_file("cl100k_base.tiktoken")) {
        auto tokens = tokenizer.encode("Hello, world!");
        // Tokens will match GPT-4's tokenization
    }
    
    // Or load from base64 encoded tiktoken data
    std::string base64_merges = "...";  // base64 encoded tiktoken merges
    if (tokenizer.load_from_base64(base64_merges)) {
        // ... use tokenizer
    }
    
    // Or load directly from binary data
    std::ifstream file("cl100k_base.tiktoken", std::ios::binary);
    std::vector<uint8_t> binary_data((std::istreambuf_iterator<char>(file)),
                                     std::istreambuf_iterator<char>());
    if (tokenizer.load_from_tiktoken_binary(binary_data)) {
        // ... use tokenizer
    }
    
    return 0;
}
```

#### GPT-2 Format

```cpp
#include "tknzr/tknzr.hpp"

int main() {
    tknzr::Tokenizer tokenizer(50257);  // GPT-2 vocabulary size
    
    // Load GPT-2 tokenizer (text format with space-separated pairs)
    if (tokenizer.load_from_file("gpt2_merges.txt")) {
        auto tokens = tokenizer.encode("Hello, world!");
        // ... use tokens
    }
    
    return 0;
}
```

### Advanced Usage

```cpp
#include "tknzr/tknzr.hpp"

int main() {
    // Create tokenizer with custom vocabulary size
    tknzr::Tokenizer tokenizer(50257);  // GPT-2 vocabulary size
    
    // Train on large corpus
    std::string corpus = "...";  // your training text
    tokenizer.train(corpus, 50257);
    
    // Get vocabulary information
    std::cout << "Vocabulary size: " << tokenizer.vocab_size() << std::endl;
    
    // Access merge rules
    const auto& merges = tokenizer.get_merges();
    for (const auto& [token_id, pair] : merges) {
        std::cout << "Token " << token_id << " = (" 
                  << pair.first << ", " << pair.second << ")" << std::endl;
    }
    
    return 0;
}
```

## API Reference

### `Tokenizer` Class

#### Constructor
```cpp
explicit Tokenizer(int vocab_size = 100256);  // Default: GPT-4/cl100k_base size
```

#### Methods

- `bool load_from_file(const std::string& filepath)`  
  Load tokenizer from a file (auto-detects tiktoken binary or base64/text format)

- `bool load_from_base64(const std::string& base64_merges)`  
  Load tokenizer from base64 encoded merges string (supports tiktoken format)

- `bool load_from_tiktoken_binary(const std::vector<uint8_t>& binary_data)`  
  Load tokenizer from tiktoken binary format (GPT-4 compatible, little-endian uint16_t pairs)

- `void train(const std::string& text, int vocab_size)`  
  Train tokenizer on text data

- `TokenList encode(const std::string& text) const`  
  Encode text into a vector of token IDs

- `std::string decode(const TokenList& tokens) const`  
  Decode tokens back to text

- `size_t vocab_size() const`  
  Get the vocabulary size

- `const std::unordered_map<Token, Pair>& get_merges() const`  
  Get the merge rules (vocabulary)

## Testing

Run tests with:

```bash
cd build
ctest
```

Or run the test executable directly:

```bash
./test_tknzr
```

## Algorithm

The library implements Byte Pair Encoding (BPE), which:

1. Starts with a base vocabulary of 256 tokens (one for each byte)
2. Finds the most frequent pair of consecutive tokens
3. Merges them into a new token
4. Repeats until desired vocabulary size is reached

This is the same algorithm used by GPT models, making the tokenizer compatible with GPT API.

## GPT-4 Compatibility

The tokenizer is fully compatible with GPT-4's tokenization:

- **Default vocabulary size**: 100256 (cl100k_base)
- **tiktoken format support**: Loads binary tiktoken files directly
- **Merge format**: Supports little-endian uint16_t pairs as used by tiktoken
- **Encoding/Decoding**: Matches GPT-4's tokenization behavior

To use with GPT-4, simply load a tiktoken file (e.g., `cl100k_base.tiktoken`) using `load_from_file()`.

## Getting tiktoken Files

You can obtain tiktoken files from:
- OpenAI's tiktoken repository
- Hugging Face model repositories
- Or generate them using the Python tiktoken library

## License

[Add your license here]

## Contributing

[Add contribution guidelines here]

