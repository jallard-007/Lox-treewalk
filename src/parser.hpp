#pragma once

#include <vector>
#include <cstdint>
#include <span>
#include <expected>
#include "token.hpp"
#include "node.hpp"
#include "allocator.hpp"

struct ParserError {};

struct Parser {
    const std::vector<Token> tokens;
    size_t current = 0;
    ASTAllocator allocator;
  
    Parser(ASTAllocator allocator, std::vector<Token> tokens);

    std::expected<std::vector<StatementNode*>, ParserError> parse();

    std::expected<StatementNode*, ParserError> parse_statement();
    std::expected<StatementNode*, ParserError> parse_print_statement();
    std::expected<StatementNode*, ParserError> parse_expression_statement();

    std::expected<ExpressionNode*, ParserError> parse_expression();
    std::expected<ExpressionNode*, ParserError> parse_equality();
    std::expected<ExpressionNode*, ParserError> parse_comparison();
    std::expected<ExpressionNode*, ParserError> parse_term();
    std::expected<ExpressionNode*, ParserError> parse_factor();
    std::expected<ExpressionNode*, ParserError> parse_unary();
    std::expected<ExpressionNode*, ParserError> parse_primary();

    bool match_token(const std::span<const TokenType>);
    bool check_next_token(TokenType);
    bool is_at_end();

    std::expected<const Token*, ParserError> consume(TokenType, std::string_view);
    const Token& advance();
    const Token& peek();
    const Token& previous();

    void synchronize();
    ParserError error(Token, std::string_view);
};
