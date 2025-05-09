#include "parser.hpp"
#include "lox.hpp"

Parser::Parser(ASTAllocator allocator, std::vector<Token> tokens): allocator{std::move(allocator)}, tokens{std::move(tokens)} {}


std::expected<std::vector<StatementNode*>, ParserError> Parser::parse() {
    std::vector<StatementNode*> statements;
    while (!this->is_at_end()) {
        auto res = this->parse_declaration();
        if (!res.has_value()) {
            return std::unexpected(std::move(res.error()));
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
    auto name = this->consume(TokenType::IDENTIFIER, "Expect variable name.");
    if (!name.has_value()) {
        return std::unexpected(name.error());
    }
    ExpressionNode* initializer = nullptr;
    if (this->match_token({{TokenType::EQUAL}})) {
        auto res = this->parse_expression();
        if (!res.has_value()) {
            return std::unexpected(res.error());
        }
        initializer = res.value();
    }

    this->consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");

    return this->allocator.create<StatementNode>(this->allocator.create<VariableDefStatementNode>(*name.value(), initializer));
}


std::expected<StatementNode*, ParserError> Parser::parse_statement() {
    if (this->match_token({{TokenType::PRINT}})) return this->parse_print_statement();

    return parse_expression_statement();
}


std::expected<StatementNode*, ParserError> Parser::parse_print_statement() {
    auto expr = this->parse_expression();
    if (!expr.has_value()) {
        return std::unexpected(std::move(expr.error()));
    }
    {
        auto res = this->consume(TokenType::SEMICOLON, "Expect ';' after value.");
        if (!res.has_value()) return std::unexpected(std::move(res.error()));
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

        if (expr.value()->get_type() == ExpressionType::Variable) {
            Token& name = expr.value()->get_variable_node()->name;
            return this->allocator.create<ExpressionNode>((this->allocator.create<AssignmentNode>(name, value.value())));
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

        expr = this->allocator.create<ExpressionNode>((this->allocator.create<BinaryNode>(oper, *expr, *right)));
    }

    return expr;
}


std::expected<ExpressionNode*, ParserError> Parser::parse_comparison() {
    auto expr = this->parse_term();
    if (!expr.has_value())
        return expr;

    while (this->match_token({{TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL}})) {
        const Token& oper = this->previous();
        auto right = this->parse_term();
        if (!right.has_value())
            return right;

        expr = this->allocator.create<ExpressionNode>((this->allocator.create<BinaryNode>(oper, *expr, *right)));
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
    if (this->match_token({{TokenType::FALSE}}))
        return this->allocator.create<ExpressionNode>(this->allocator.create<LiteralNode>(false));
    if (this->match_token({{TokenType::TRUE}}))
        return this->allocator.create<ExpressionNode>(this->allocator.create<LiteralNode>(true));
    if (this->match_token({{TokenType::NIL}}))
        return this->allocator.create<ExpressionNode>(this->allocator.create<LiteralNode>(None()));

    if (this->match_token({{TokenType::NUMBER, TokenType::STRING}})) {
        return this->allocator.create<ExpressionNode>(this->allocator.create<LiteralNode>(this->previous().val));
    }

    if (this->match_token({{TokenType::IDENTIFIER}})) {
        return this->allocator.create<ExpressionNode>(this->allocator.create<VariableNode>(this->previous()));
    }

    if (this->match_token({{TokenType::LEFT_PAREN}})) {
        auto expr = this->parse_expression();
        if (!expr.has_value())
            return expr;

        auto res = this->consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        if (!res.has_value())
            return std::unexpected(res.error());  

        return expr;
    }

    return std::unexpected(this->error(this->peek(), "Expect expression.")); 
}


bool Parser::match_token(const std::span<const TokenType> types) {
    for (TokenType type : types) {
        if (this->check_next_token(type)) {
            this->advance();
            return true;
        }
    }

    return false;
}


bool Parser::check_next_token(TokenType type) {
    if (this->is_at_end()) return false;
    return this->peek().type == type;
}


bool Parser::is_at_end() {
    return this->peek().type == TokenType::END_OF_FILE;
}


std::expected<const Token*, ParserError> Parser::consume(TokenType t, std::string_view v) {
    // if (this->check_next_token(t)) return std::expected<const Token*, ParserError>(&this->advance());
    if (this->check_next_token(t)) return &this->advance();
    this->error(this->peek(), v);
    return std::unexpected(ParserError());
}


const Token& Parser::advance() {
    if (!this->is_at_end()) this->current++;
    return this->previous();
}


const Token& Parser::peek() {
    return this->tokens[this->current];
}


const Token& Parser::previous() {
    return this->tokens[this->current - 1];
}

void Parser::synchronize() {
    this->advance();

    while (!this->is_at_end()) {
        if (this->previous().type == TokenType::SEMICOLON) return;

        switch (this->peek().type) {
            case TokenType::CLASS:
            case TokenType::FUN:
            case TokenType::VAR:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::PRINT:
            case TokenType::RETURN:
                return;
            default:
                break;
        }

        this->advance();
    }
}

ParserError Parser::error(Token token, std::string_view message) {
    Lox::error(token, message);
    return ParserError();
}
