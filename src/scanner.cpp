
#include <cstring>
#include "token.hpp"
#include "perfect_hash.hpp"
#include "scanner.hpp"
#include "lox.hpp"


bool is_digit(char c) {
    return c >= '0' && c <= '9';
}


bool is_alpha(char c) {
    return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_';
}


bool is_alpha_numeric(char c) {
    return is_digit(c) || is_alpha(c);
}


Scanner::Scanner(std::string program): program_{std::move(program)}, program{this->program_} {}


std::vector<Token> Scanner::scan() {
    while (!this->check_at_end()) {
        this->start = this->current;
        this->scan_next();
    }
    this->start = this->current;
    tokens.emplace_back(Token{TokenType::END_OF_FILE, "EOF", None(), this->line});
    return tokens;
}


void Scanner::scan_next() {
    char c = this->advance();
    switch (c) {
        case '(': this->add_token(TokenType::LEFT_PAREN); break;
        case ')': this->add_token(TokenType::RIGHT_PAREN); break;
        case '{': this->add_token(TokenType::LEFT_BRACE); break;
        case '}': this->add_token(TokenType::RIGHT_BRACE); break;
        case ',': this->add_token(TokenType::COMMA); break;
        case '.': this->add_token(TokenType::DOT); break;
        case '-': this->add_token(TokenType::MINUS); break;
        case '+': this->add_token(TokenType::PLUS); break;
        case ';': this->add_token(TokenType::SEMICOLON); break;
        case '*': this->add_token(TokenType::STAR); break; 
        case '!':
            this->add_token(this->match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
            break;
        case '=':
            this->add_token(this->match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
            break;
        case '<':
            this->add_token(this->match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
            break;
        case '>':
            this->add_token(this->match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
            break;
        case '/':
            if (this->match('/')) {
                // A comment goes until the end of the line.
                while (this->peek() != '\n' && !this->check_at_end()) this->advance();
            } else {
                this->add_token(TokenType::SLASH);
            }
            break;
        case '"': this->handle_string(); break;
        case ' ':
        case '\r':
        case '\t':
            // Ignore whitespace.
            break;
    
        case '\n':
            line++;
            break;
        default:
            if (is_digit(c)) {
                this->handle_number();
            }
            else if (is_alpha(c)) {
                this->handle_identifier();
            } else {
                Lox::error(this->line, "Unexpected character.");
            }
            break;
    }
}


void Scanner::handle_string() {
    while (this->peek() != '"' && !this->check_at_end()) {
        if (this->peek() == '\n') line++;
        this->advance();
    }

    if (this->check_at_end()) {
        Lox::error(line, "Unterminated string.");
        return;
    }

    // The closing ".
    this->advance();

    // Trim the surrounding quotes.
    auto size = (this->current - 1) - (this->start + 1);
    std::string_view v = this->program.substr(start + 1, size);
    this->add_token(TokenType::STRING, v);
}

void Scanner::handle_number() {
    while (is_digit(this->peek())) this->advance();

    // Look for a fractional part.
    if (this->peek() == '.' && isdigit(this->peek_next())) {
        // Consume the "."
        this->advance();

        while (is_digit(this->peek())) this->advance();
    }
    uint32_t len = this->current - this->start;
    double num = std::stod(std::string(this->program.substr(this->start, len)));
    this->add_token(TokenType::NUMBER, num);
}


void Scanner::handle_identifier() {
    while (is_alpha_numeric(this->peek())) this->advance();

    uint32_t len = current - start;
    std::string_view v = this->program.substr(start, len);
    auto kwt = this->get_keyword_type(v);
    if (kwt) {
        this->add_token(kwt.value());
    } else {
        this->add_token(TokenType::IDENTIFIER);
    }
}


std::optional<TokenType> Scanner::get_keyword_type(std::string_view word) {
    auto t = KeywordLookup::lookup_keyword(word.data(), word.length());
    if (t) {
        return std::optional<TokenType>(t->token);
    }
    return std::nullopt;
}


char Scanner::peek() {
    if (this->check_at_end()) return '\0';
    return this->program[current];
}


char Scanner::peek_next() {
    if (this->current + 1 >= this->program.length()) return '\0';
    return this->program[this->current + 1];
} 


bool Scanner::match(char expected) {
    if (this->check_at_end()) return false;
    if (program[current] != expected) return false;
    current++;
    return true;
}


char Scanner::advance() {
    return this->program[this->current++];
}


void Scanner::add_token(TokenType type) {
    add_token(type, None());
}


void Scanner::add_token(TokenType type, Object literal) {
    auto lexeme = program.substr(start, current - start);
    tokens.emplace_back(Token{type, lexeme, literal, line});
}


bool Scanner::check_at_end() {
    return this->current >= this->program.length();
}

const std::unordered_map<std::string_view, TokenType> Scanner::keywords = {
    {"and",    TokenType::AND},
    {"class",  TokenType::CLASS},
    {"else",   TokenType::ELSE},
    {"false",  TokenType::FALSE},
    {"for",    TokenType::FOR},
    {"fun",    TokenType::FUN},
    {"if",     TokenType::IF},
    {"nil",    TokenType::NIL},
    {"or",     TokenType::OR},
    {"print",  TokenType::PRINT},
    {"return", TokenType::RETURN},
    {"super",  TokenType::SUPER},
    {"this",   TokenType::THIS},
    {"true",   TokenType::TRUE},
    {"var",    TokenType::VAR},
    {"while",  TokenType::WHILE}
};
