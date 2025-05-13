#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <unordered_map>
#include <string>
#include <string_view>

#include "token.hpp"

struct Scanner {

    static const std::unordered_map<std::string_view, TokenType> keywords;

    const std::string program_;
    const std::string_view program;
    
    std::vector<Token> tokens;

    uint32_t start = 0;
    uint32_t current = 0;
    uint32_t line = 1;

    explicit Scanner(std::string program);

    std::vector<Token> scan();

    void scan_next();

    void handle_string();

    void handle_number();

    void handle_identifier();

    bool match(char expected);

    char peek();
    char peek_next() const;

    char advance();

    void add_token(TokenType type);

    void add_token(TokenType type, Object literal);

    bool check_at_end() const;

    std::optional<TokenType> get_keyword_type(std::string_view word);

};