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
    if (this->match_token({{FOR}})) return this->parse_for_statement();
    if (this->match_token({{IF}})) return this->parse_if_statement();
    if (this->match_token({{PRINT}})) return this->parse_print_statement();
    if (this->match_token({{LEFT_BRACE}})) return this->parse_block();
    if (this->match_token({{BREAK}})) return this->parse_break_statement();
    if (this->match_token({{RETURN}})) return this->parse_return_statement();
    if (this->match_token({{WHILE}})) return this->parse_while_statement();

    return parse_expression_statement();
}


std::expected<StatementNode*, ParserError> Parser::parse_block() {
    auto statements = this->allocator.create<std::vector<StatementNode*>>();
    while (!this->check_next_token(TokenType::RIGHT_BRACE) && !this->is_at_end()) {
        auto res = this->parse_declaration();
        if (!res.has_value()) {
            return std::unexpected(std::move(res.error()));
        }
        statements->push_back(res.value());
    }
    if (auto res = this->consume(TokenType::RIGHT_BRACE, "Expect '}' after block."); !res.has_value()) {
        return std::unexpected(res.error());
    }

    return this->allocator.create<StatementNode>(this->allocator.create<BlockStatementNode>(statements));
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
        if (!res.has_value()) return std::unexpected(res.error());
    }
    return this->allocator.create<StatementNode>(this->allocator.create<ExpressionStatementNode>(expr.value()));
}


std::expected<StatementNode*, ParserError> Parser::parse_if_statement() {
    if (auto res = this->consume(LEFT_PAREN, "Expect '(' after 'if'."); !res.has_value()) {
        return std::unexpected(res.error());
    }
    auto condition = this->parse_expression();
    if (auto res = this->consume(RIGHT_PAREN, "Expect ')' after if condition."); !res.has_value()) {
        return std::unexpected(res.error());
    }

    auto then_branch = this->parse_statement();
    if (!then_branch.has_value()) {
        return then_branch;
    }

    StatementNode* else_branch = nullptr;
    if (this->match_token({{TokenType::ELSE}})) {
        auto else_branch_res = this->parse_statement();
        if (!else_branch_res.has_value()) {
            return else_branch_res;
        }
        else_branch = else_branch_res.value();
    }

    return this->allocator.create<StatementNode>(this->allocator.create<IfStatementNode>(condition.value(), then_branch.value(), else_branch));
}

std::expected<StatementNode*, ParserError> Parser::parse_while_statement() {
    if (auto res = this->consume(LEFT_PAREN, "Expect '(' after 'while'."); !res.has_value()) {
        return std::unexpected(res.error());
    }
    auto condition = this->parse_expression();
    if (!condition.has_value()) {
        return std::unexpected(condition.error());
    }
    if (auto res = this->consume(RIGHT_PAREN, "Expect ')' after condition."); !res.has_value()) {
        return std::unexpected(res.error());
    }
    this->loop_depth++;
    auto body = this->parse_statement();
    this->loop_depth--;
    if (!body.has_value()) {
        return body;
    }

    return this->allocator.create<StatementNode>(this->allocator.create<WhileStatementNode>(condition.value(), body.value()));
}


std::expected<StatementNode*, ParserError> Parser::parse_for_statement() {
    if (auto res = this->consume(LEFT_PAREN, "Expect '(' after 'for'."); !res.has_value()) {
        return std::unexpected(res.error());
    }
    StatementNode* initializer;
    if (this->match_token({{SEMICOLON}})) {
        initializer = nullptr;
    } else if (this->match_token({{VAR}})) {
        auto ini_res = this->parse_variable_declaration();
        if (!ini_res.has_value()) {
            return ini_res;
        }
        initializer = ini_res.value();
    } else {
        auto ini_res = this->parse_expression_statement();
        if (!ini_res.has_value()) {
            return ini_res;
        }
        initializer = ini_res.value();
    }

    ExpressionNode* condition = nullptr;
    if (!this->check_next_token(SEMICOLON)) {
        auto expr_res = this->parse_expression();
        if (!expr_res.has_value()) {
            return std::unexpected(expr_res.error());
        }
        condition = expr_res.value();
    }

    if (auto res = this->consume(SEMICOLON, "Expect ';' after loop condition."); !res.has_value()) {
        return std::unexpected(res.error());
    }

    ExpressionNode* increment = nullptr;
    if (!this->check_next_token(SEMICOLON)) {
        auto expr_res = this->parse_expression();
        if (!expr_res.has_value()) {
            return std::unexpected(expr_res.error());
        }
        increment = expr_res.value();
    }

    if (auto res = this->consume(RIGHT_PAREN, "Expect ')' after for clauses."); !res.has_value()) {
        return std::unexpected(res.error());
    }

    this->loop_depth++;
    auto body_exp = this->parse_statement();
    this->loop_depth--;
    if (!body_exp.has_value()) {
        return body_exp;
    }
    StatementNode* body = body_exp.value();
    if (increment) {
        auto increment_stmt = this->allocator.create<StatementNode>(this->allocator.create<ExpressionStatementNode>(increment));
        auto st_list = this->allocator.create<std::vector<StatementNode*>>(std::vector<StatementNode*>({{body, increment_stmt}}));
        body = this->allocator.create<StatementNode>(this->allocator.create<BlockStatementNode>(st_list));
    }

    if (!condition) {
        condition = this->allocator.create<ExpressionNode>(this->allocator.create<LiteralNode>(true));
    }
    auto while_stmt = this->allocator.create<StatementNode>(this->allocator.create<WhileStatementNode>(condition, body));


    if (initializer) {
        auto st_list = this->allocator.create<std::vector<StatementNode*>>(std::vector<StatementNode*>({{initializer, body}}));
        while_stmt = this->allocator.create<StatementNode>(this->allocator.create<BlockStatementNode>(st_list));
    }

    return while_stmt;
}


std::expected<StatementNode*, ParserError> Parser::parse_break_statement() {
    if (this->loop_depth == 0) {
        this->error(this->previous(), "");
        return std::unexpected(ParserError{});
    }
    if (auto res = this->consume(SEMICOLON, "Expect ';' after 'break'."); !res.has_value()) {
        return std::unexpected(res.error());
    }
    return this->allocator.create<StatementNode>(this->allocator.create<BreakStatementNode>());
}


std::expected<StatementNode*, ParserError> Parser::parse_return_statement() {
    const Token& rt = this->previous();
    ExpressionNode* rt_exp = nullptr;
    if (!this->check_next_token(SEMICOLON)) {
        auto rt_res = this->parse_expression_statement();
        if (!rt_res.has_value()) {
            return std::unexpected(rt_res.error());
        }
        rt_exp = rt_res.value()->get_expression_statement_node()->expr;
    } else {
        this->advance();
    }
    return this->allocator.create<StatementNode>(this->allocator.create<ReturnStatementNode>(rt, rt_exp));
}


std::expected<ExpressionNode*, ParserError> Parser::parse_expression() {
    return this->parse_assignment();
}


std::expected<ExpressionNode*, ParserError> Parser::parse_assignment() {
    auto expr = this->parse_logical_or();
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


std::expected<ExpressionNode*, ParserError> Parser::parse_logical_or() {
    auto expr_exp = this->parse_logical_and();
    if (!expr_exp.has_value())
        return expr_exp;

    ExpressionNode* expr = expr_exp.value();
    while (this->match_token({{OR}})) {
        auto& oper = this->previous();
        auto right = this->parse_logical_and();
        if (!right.has_value()) {
            return right;
        }
        expr = this->allocator.create<ExpressionNode>(this->allocator.create<LogicalNode>(oper, expr, right.value()));
    }

    return expr;
}


std::expected<ExpressionNode*, ParserError> Parser::parse_logical_and() {
    auto expr_exp = this->parse_equality();
    if (!expr_exp.has_value())
        return expr_exp;
    
    ExpressionNode* expr = expr_exp.value();
    while (this->match_token({{AND}})) {
        auto& oper = this->previous();
        auto right = this->parse_equality();
        if (!right.has_value()) {
            return right;
        }
        expr = this->allocator.create<ExpressionNode>(this->allocator.create<LogicalNode>(oper, expr, right.value()));
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
    auto expr_exp = this->parse_unary();
    if (!expr_exp.has_value())
        return expr_exp;

    ExpressionNode* expr = expr_exp.value();
    while (this->match_token({{TokenType::MINUS, TokenType::PLUS}})) {
        const Token& oper = previous();
        auto right = this->parse_unary();
        if (!right.has_value())
            return right;

        expr = this->allocator.create<ExpressionNode>(this->allocator.create<BinaryNode>(oper, expr, *right));
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
  
    return this->parse_call();
}


std::expected<ExpressionNode*, ParserError> Parser::parse_call() {
    auto expr_exp = this->parse_primary();
    if (!expr_exp.has_value()) {
        return expr_exp;
    }
    
    ExpressionNode* expr = expr_exp.value();
    while (true) { 
        if (this->match_token({{LEFT_PAREN}})) {
            auto res = this->finish_call(*expr);
            if (!res.has_value()) {
                return res;
            }
            expr = res.value();
        } else {
            break;
        }
    }

    return expr;
}

std::expected<ExpressionNode*, ParserError> Parser::finish_call(ExpressionNode& callee) {
    auto arguments = this->allocator.create<std::vector<ExpressionNode*>>();
    if (!this->check_next_token(RIGHT_PAREN)) {
        do {
            if (arguments->size() >= 255) {
                this->error(this->peek(), "Can't have more than 255 arguments.");
            }
            auto expr = this->parse_expression();
            if (!expr.has_value()) {
                return expr;
            }
            arguments->push_back(expr.value());
        } while (this->match_token({{COMMA}}));
    }

    auto paren_exp = this->consume(RIGHT_PAREN, "Expect ')' after arguments.");
    if (!paren_exp.has_value()) {
        return std::unexpected(paren_exp.error());
    }

    return this->allocator.create<ExpressionNode>(this->allocator.create<CallNode>(&callee, *paren_exp.value(), arguments));
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
