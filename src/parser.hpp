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
    uint32_t loop_depth = 0;
  
    Parser(ASTAllocator allocator, std::vector<Token> tokens);

    [[nodiscard]] std::expected<std::vector<StatementNode*>, ParserError> parse();

    [[nodiscard]] std::expected<StatementNode*, ParserError> parse_declaration();
    [[nodiscard]] std::expected<StatementNode*, ParserError> parse_declaration2();
    
    [[nodiscard]] std::expected<StatementNode*, ParserError> parse_block();

    [[nodiscard]] std::expected<StatementNode*, ParserError> parse_variable_declaration();

    [[nodiscard]] std::expected<StatementNode*, ParserError> parse_statement();
    [[nodiscard]] std::expected<StatementNode*, ParserError> parse_print_statement();
    [[nodiscard]] std::expected<StatementNode*, ParserError> parse_expression_statement();
    [[nodiscard]] std::expected<StatementNode*, ParserError> parse_if_statement();
    [[nodiscard]] std::expected<StatementNode*, ParserError> parse_while_statement();
    [[nodiscard]] std::expected<StatementNode*, ParserError> parse_for_statement();
    [[nodiscard]] std::expected<StatementNode*, ParserError> parse_break_statement();
    [[nodiscard]] std::expected<StatementNode*, ParserError> parse_return_statement();
    
    [[nodiscard]] std::expected<ExpressionNode*, ParserError> parse_expression();
    [[nodiscard]] std::expected<ExpressionNode*, ParserError> parse_assignment();
    [[nodiscard]] std::expected<ExpressionNode*, ParserError> parse_logical_or();
    [[nodiscard]] std::expected<ExpressionNode*, ParserError> parse_logical_and();
    [[nodiscard]] std::expected<ExpressionNode*, ParserError> parse_equality();
    [[nodiscard]] std::expected<ExpressionNode*, ParserError> parse_comparison();
    [[nodiscard]] std::expected<ExpressionNode*, ParserError> parse_term();
    [[nodiscard]] std::expected<ExpressionNode*, ParserError> parse_factor();
    [[nodiscard]] std::expected<ExpressionNode*, ParserError> parse_unary();
    [[nodiscard]] std::expected<ExpressionNode*, ParserError> parse_call();
    [[nodiscard]] std::expected<ExpressionNode*, ParserError> finish_call(ExpressionNode&);
    [[nodiscard]] std::expected<ExpressionNode*, ParserError> parse_primary();


    bool match_token(const std::span<const TokenType>);
    bool check_next_token(TokenType) const;
    bool is_at_end() const;


    std::expected<const Token*, ParserError> consume(TokenType, std::string_view);
    const Token& advance();
    const Token& peek() const;
    const Token& previous() const;

    void synchronize();
    ParserError error(const Token&, std::string_view) const;
};
