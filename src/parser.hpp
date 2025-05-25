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
    std::vector<Token>& tokens;
    ASTAllocator& allocator;
    std::vector<StatementNode*>& statements;
    size_t current = 0;

    Parser(Program&);

    void parse();

    [[nodiscard]] StatementNode* parse_declaration();
    [[nodiscard]] std::expected<StatementNode*, ParserError> parse_declaration2();

    [[nodiscard]] std::expected<BlockStatementNode*, ParserError> parse_block();

    [[nodiscard]] std::expected<VariableDeclarationNode*, ParserError> parse_variable_declaration();
    [[nodiscard]] std::expected<FunctionDeclarationNode*, ParserError> parse_function_declaration(std::string_view);
    [[nodiscard]] std::expected<ClassDeclarationNode*, ParserError> parse_class_declaration();

    [[nodiscard]] std::expected<StatementNode*, ParserError> parse_statement();
    [[nodiscard]] std::expected<PrintStatementNode*, ParserError> parse_print_statement();
    [[nodiscard]] std::expected<ExpressionStatementNode*, ParserError> parse_expression_statement();
    [[nodiscard]] std::expected<IfStatementNode*, ParserError> parse_if_statement();
    [[nodiscard]] std::expected<WhileStatementNode*, ParserError> parse_while_statement();
    [[nodiscard]] std::expected<BlockStatementNode*, ParserError> parse_for_statement();
    [[nodiscard]] std::expected<BreakStatementNode*, ParserError> parse_break_statement();
    [[nodiscard]] std::expected<ReturnStatementNode*, ParserError> parse_return_statement();

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


    std::expected<Token*, ParserError> consume(TokenType, std::string_view);
    Token& advance();
    Token& peek() const;
    Token& previous() const;

    void synchronize();
    ParserError error(const Token&, std::string_view) const;
};
