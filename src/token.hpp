#pragma once

#include <cstdint>
#include <compare>
#include <array>
#include <utility>
#include <variant>
#include <string_view>
#include <string>
#include <iostream>


enum class TokenType: uint8_t {
    AND,
    BANG_EQUAL,
    BANG,
    BREAK,
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
using String = std::string;

struct LoxCallable;

using Object = std::variant<None, Number, String, bool, LoxCallable*>;

struct Interpreter;

class LoxCallable {
public:
    virtual int arity() = 0;
    virtual Object call(Interpreter*, std::vector<Object>&) = 0;
    virtual ~LoxCallable() = default;
};

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


constexpr std::array<const char *, std::to_underlying(TokenType::_COUNT)> token_map = {
    "and",
    "bang_equal",
    "bang",
    "break",
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