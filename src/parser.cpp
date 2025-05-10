#include <algorithm>
#include "parser.hpp"
#include "lox.hpp"


using enum TokenType;


Parser::Parser(ASTAllocator allocator, std::vector<Token> tokens): tokens{std::move(tokens)}, allocator{std::move(allocator)} {}


std::expected<std::vector<StatementNode*>, ParserError> Parser::parse() {
    std::vector<StatementNode*> statements;
    while (!this->is_at_end()) {
        auto res = this->parse_declaration();
        if (!res.has_value()) {
            return std::unexpected(res.error());
        }
        statements.push_back(res.value());
    }
    return statements;
}


std::expected<StatementNode*, ParserError> Parser::parse_declaration() {
    std::expected<StatementNode*, ParserError> res = this->parse_declaration2();
    if (!res.has_value()) {
        this->synchronize();
    }
    return res;
}


std::expected<StatementNode*, ParserError> Parser::parse_declaration2() {
    if (this->match_token({{TokenType::VAR}})) {
        return this->parse_variable_declaration();
    } else {
        return this->parse_statement();
    }
}


std::expected<StatementNode*, ParserError> Parser::parse_variable_declaration() {
    auto name = this->consume(IDENTIFIER, "Expect variable name.");
    if (!name.has_value()) {
        return std::unexpected(name.error());
    }
    ExpressionNode* initializer = nullptr;
    if (this->match_token({{EQUAL}})) {
        auto res = this->parse_expression();
        if (!res.has_value()) {
            return std::unexpected(res.error());
        }
        initializer = res.value();
    }

    this->consume(SEMICOLON, "Expect ';' after variable declaration.");

    return this->allocator.create<StatementNode>(this->allocator.create<VariableDefStatementNode>(*name.value(), initializer));
}


std::expected<StatementNode*, ParserError> Parser::parse_statement() {
    if (this->match_token({{TokenType::PRINT}})) return this->parse_print_statement();
    if (this->match_token({{TokenType::LEFT_BRACE}})) return this->parse_block();

    return parse_expression_statement();
}


std::expected<StatementNode*, ParserError> Parser::parse_block() {
    std::vector<StatementNode*> statements;
    while (!this->check_next_token(TokenType::RIGHT_BRACE) && !this->is_at_end()) {
        auto res = this->parse_declaration();
        if (!res.has_value()) {
            return std::unexpected(std::move(res.error()));
        }
        statements.push_back(res.value());
    }
    if (auto res = this->consume(TokenType::RIGHT_BRACE, "Expect '}' after block."); !res.has_value()) {
        return std::unexpected(res.error());
    }

    return this->allocator.create<StatementNode>(this->allocator.create<BlockStatementNode>(std::move(statements)));
}


std::expected<StatementNode*, ParserError> Parser::parse_print_statement() {
    auto expr = this->parse_expression();
    if (!expr.has_value()) {
        return std::unexpected(std::move(expr.error()));
    }
    if (auto res = this->consume(TokenType::SEMICOLON, "Expect ';' after value."); !res.has_value()) {
        return std::unexpected(std::move(res.error()));
    }
    return this->allocator.create<StatementNode>(this->allocator.create<PrintStatementNode>(expr.value()));
}


std::expected<StatementNode*, ParserError> Parser::parse_expression_statement() {
    auto expr = this->parse_expression();
    if (!expr.has_value()) {
        return std::unexpected(std::move(expr.error()));
    }
    {
        auto res = this->consume(TokenType::SEMICOLON, "Expect ';' after value.");
        if (!res.has_value()) return std::unexpected(std::move(res.error()));
    }
    return this->allocator.create<StatementNode>(this->allocator.create<ExpressionStatementNode>(expr.value()));
}


std::expected<ExpressionNode*, ParserError> Parser::parse_expression() {
    return this->parse_assignment();
}


std::expected<ExpressionNode*, ParserError> Parser::parse_assignment() {
    auto expr = this->parse_equality();
    if (!expr.has_value())
        return expr;

    if (this->match_token({{TokenType::EQUAL}})) {
        const Token& equals = this->previous();
        auto value = this->parse_assignment();
        if (!value.has_value())
            return value;

        if (expr.value()->get_type() == ExpressionType::VARIABLE) {
            Token& name = expr.value()->get_variable_node()->name;
            return this->allocator.create<ExpressionNode>(this->allocator.create<AssignmentNode>(name, value.value()));
        }

        this->error(equals, "Invalid assignment target."); 
    }

    return expr;
}


std::expected<ExpressionNode*, ParserError> Parser::parse_equality() {
    auto expr = this->parse_comparison();
    if (!expr.has_value())
        return expr;

    while (this->match_token({{TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL}})) {
        const Token& oper = this->previous();
        auto right = this->parse_comparison();
        if (!right.has_value())
            return right;

        expr = this->allocator.create<ExpressionNode>(this->allocator.create<BinaryNode>(oper, *expr, *right));
    }

    return expr;
}


std::expected<ExpressionNode*, ParserError> Parser::parse_comparison() {
    auto expr = this->parse_term();
    if (!expr.has_value())
        return expr;

    while (this->match_token({{GREATER, GREATER_EQUAL, LESS, LESS_EQUAL}})) {
        const Token& oper = this->previous();
        auto right = this->parse_term();
        if (!right.has_value())
            return right;

        expr = this->allocator.create<ExpressionNode>(this->allocator.create<BinaryNode>(oper, *expr, *right));
    }

    return expr;
}


std::expected<ExpressionNode*, ParserError> Parser::parse_term() {
    auto expr = this->parse_factor();
    if (!expr.has_value())
        return expr;

    while (this->match_token({{TokenType::MINUS, TokenType::PLUS}})) {
        const Token& oper = previous();
        auto right = this->parse_factor();
        if (!right.has_value())
            return right;   
 
        expr = this->allocator.create<ExpressionNode>(this->allocator.create<BinaryNode>(oper, *expr, *right));
    }

    return expr;
}


std::expected<ExpressionNode*, ParserError> Parser::parse_factor() {
    auto expr = this->parse_unary();
    if (!expr.has_value())
        return expr;

    while (this->match_token({{TokenType::MINUS, TokenType::PLUS}})) {
        const Token& oper = previous();
        auto right = this->parse_unary();
        if (!right.has_value())
            return right;

        expr = this->allocator.create<ExpressionNode>(this->allocator.create<BinaryNode>(oper, *expr, *right));
    }

    return expr;
}


std::expected<ExpressionNode*, ParserError> Parser::parse_unary() {
    if (this->match_token({{TokenType::BANG, TokenType::MINUS}})) {
        const Token& oper = this->previous();
        auto right = this->parse_primary();
        if (!right.has_value())
            return right;

        return this->allocator.create<ExpressionNode>(this->allocator.create<UnaryNode>(oper, *right));
    }
  
    return this->parse_primary();
}


std::expected<ExpressionNode*, ParserError> Parser::parse_primary() {
    if (this->match_token({{FALSE}}))
        return this->allocator.create<ExpressionNode>(this->allocator.create<LiteralNode>(false));
    if (this->match_token({{TRUE}}))
        return this->allocator.create<ExpressionNode>(this->allocator.create<LiteralNode>(true));
    if (this->match_token({{NIL}}))
        return this->allocator.create<ExpressionNode>(this->allocator.create<LiteralNode>(None()));

    if (this->match_token({{NUMBER, STRING}})) {
        return this->allocator.create<ExpressionNode>(this->allocator.create<LiteralNode>(this->previous().val));
    }

    if (this->match_token({{IDENTIFIER}})) {
        return this->allocator.create<ExpressionNode>(this->allocator.create<VariableNode>(this->previous()));
    }

    if (this->match_token({{LEFT_PAREN}})) {
        auto expr = this->parse_expression();
        if (!expr.has_value())
            return expr;

        if (auto res = this->consume(RIGHT_PAREN, "Expect ')' after expression."); !res.has_value())
            return std::unexpected(res.error());  

        return expr;
    }

    return std::unexpected(this->error(this->peek(), "Expect expression.")); 
}


bool Parser::match_token(const std::span<const TokenType> types) {
    if (std::ranges::any_of(types, [&](TokenType type) { return this->check_next_token(type); })) {
        this->advance();
        return true;
    }
    return false;
}


bool Parser::check_next_token(TokenType type) const {
    if (this->is_at_end()) return false;
    return this->peek().type == type;
}


bool Parser::is_at_end() const {
    return this->peek().type == TokenType::END_OF_FILE;
}


std::expected<const Token*, ParserError> Parser::consume(TokenType t, std::string_view v) {
    if (this->check_next_token(t)) return &this->advance();
    this->error(this->peek(), v);
    return std::unexpected(ParserError());
}


const Token& Parser::advance() {
    if (!this->is_at_end()) this->current++;
    return this->previous();
}


const Token& Parser::peek() const {
    return this->tokens[this->current];
}


const Token& Parser::previous() const {
    return this->tokens[this->current - 1];
}

void Parser::synchronize() {
    this->advance();

    while (!this->is_at_end()) {
        using enum TokenType;
        if (this->previous().type == SEMICOLON) return;

        switch (this->peek().type) {
            case CLASS:
            case FUN:
            case VAR:
            case FOR:
            case IF:
            case WHILE:
            case PRINT:
            case RETURN:
                return;
            default:
                break;
        }

        this->advance();
    }
}

ParserError Parser::error(const Token& token, std::string_view message) const {
    Lox::error(token, message);
    return ParserError();
}
