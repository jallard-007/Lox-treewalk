#pragma once

#include <cstdint>
#include <compare>
#include <array>
#include <variant>
#include <string_view>
#include <string>
#include <iostream>


enum class TokenType: uint8_t {
    AND,
    BANG_EQUAL,
    BANG,
    CLASS,
    COMMA,
    DOT,
    ELSE,
    END_OF_FILE,
    EQUAL_EQUAL,
    EQUAL,
    FALSE,
    FOR,
    FUN,
    GREATER_EQUAL,
    GREATER,
    IDENTIFIER,
    IF,
    LEFT_BRACE,
    LEFT_PAREN,
    LESS_EQUAL,
    LESS,
    MINUS,
    NIL,
    NUMBER,
    OR,
    PLUS,
    PRINT,
    RETURN,
    RIGHT_BRACE,
    RIGHT_PAREN,
    SEMICOLON,
    SLASH,
    STAR,
    STRING,
    SUPER,
    THIS,
    TRUE,
    VAR,
    WHILE,

    _COUNT
};

struct None {
    constexpr auto operator<=>(const None&) const = default;
};

using Number = double;
using String = std::string_view;

using Object = std::variant<None, Number, String, bool>;

struct Token {
    TokenType type = TokenType::AND;
    std::string_view lexeme {};
    Object val = None();
    uint32_t line = 0;
};


std::ostream& operator<<(std::ostream& os, const Token& t);
std::ostream& operator<<(std::ostream& os, const None& t);
std::ostream& operator<<(std::ostream& os, const Object& v);
std::ostream& operator<<(std::ostream& os, const TokenType& v);


constexpr std::array<const char *, static_cast<size_t>(TokenType::_COUNT)> token_map = {
    "and",
    "bang_equal",
    "bang",
    "class",
    "comma",
    "dot",
    "else",
    "end_of_file",
    "equal_equal",
    "equal",
    "false",
    "for",
    "fun",
    "greater_equal",
    "greater",
    "identifier",
    "if",
    "left_brace",
    "left_paren",
    "less_equal",
    "less",
    "minus",
    "nil",
    "number",
    "or",
    "plus",
    "print",
    "return",
    "right_brace",
    "right_paren",
    "semicolon",
    "slash",
    "star",
    "string",
    "super",
    "this",
    "true",
    "var",
    "while",
};