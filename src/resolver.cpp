#include "resolver.hpp"
#include "lox.hpp"
#include <stdexcept>

Resolver::Resolver(Interpreter& inter): interpreter{inter} {}


void Resolver::block(BlockStatementNode& block) {
    this->begin_scope();
    this->resolve(*block.stmts);
    this->end_scope();
}


void Resolver::begin_scope() {
    this->scopes.emplace_back();
}

void Resolver::end_scope() {
    this->scopes.pop_back();
}


void Resolver::resolve(std::vector<StatementNode*>& stmts) {
    for (auto stmt : stmts) {
        this->resolve(*stmt);
    }
}


void Resolver::resolve(StatementNode& stmt) {
    switch (stmt.get_type()) {
        case StatementType::PRINT: { }
        case StatementType::EXPRESSION: { }
        case StatementType::VARIABLE: { }
        case StatementType::BLOCK: { }
        case StatementType::IF: { }
        case StatementType::WHILE: { }
        case StatementType::BREAK: { }
        case StatementType::RETURN: { }
        case StatementType::FUNCTION: { }
    }
}


void Resolver::resolve(ExpressionNode& expr) {
    switch (expr.get_type()) {
        case ExpressionType::BINARYOP: { }
        case ExpressionType::UNARYOP: { }
        case ExpressionType::LITERAL: { }
        case ExpressionType::VARIABLE: { }
        case ExpressionType::ASSIGNMENT: { }
        case ExpressionType::LOGICAL: { }
        case ExpressionType::CALL: { }
    }
}


void Resolver::visit_var_dec_node(VariableDeclarationNode& var_dec) {
    this->declare(*var_dec.name);
    if (var_dec.initializer) {
        this->resolve(*var_dec.initializer);
    }
    this->define(*var_dec.name);
}


void Resolver::declare(Token& tk) {
    if (this->scopes.empty()) {
        return;
    }
    this->scopes.back()[std::string(tk.lexeme)] = false;
}


void Resolver::define(Token& tk) {
    auto& s = this->scopes.back();
    auto d = s.find(tk.lexeme);
    if (d == s.end()) {
        throw std::runtime_error("Variable defined but not declared");
    }
    d->second = true;
}


void Resolver::visit_var_expr(VariableNode& var_expr) {
    if (!this->scopes.empty()) {
        auto& s = this->scopes.back();
        auto d = s.find(var_expr.name->lexeme);
        if (d != s.end() && d->second == false) {
            Lox::error(*var_expr.name, "Can't read local variable in its own initializer.");
        }
    }
    ExpressionNode t {&const_cast<VariableNode&>(var_expr)};
    this->resolve_local(t, *var_expr.name);
}


void Resolver::resolve_local(ExpressionNode& expr, Token& name) {
    for (int i = this->scopes.size() - 1; i >= 0; i--) {
        if (this->scopes[i].contains(name.lexeme)) {
            this->interpreter.resolve(expr, this->scopes.size() - 1 - i);
            return;
        }
    }
}

void Resolver::visit_assign_expr(AssignmentNode& expr) {
    ExpressionNode t {&expr};
    this->resolve(*expr.expr);
    this->resolve_local(t, *expr.name);
}

void Resolver::visit_function_dec(FunctionDeclarationNode& func_dec) {
    this->declare(*func_dec.name);
    this->define(*func_dec.name);
    this->resolve_function(func_dec);
}

void Resolver::resolve_function(FunctionDeclarationNode& func_dec) {
    this->begin_scope();
    if (func_dec.params){
        for (auto& p : *func_dec.params) {
            this->declare(*p);
            this->define(*p);
        }
    }
    StatementNode s {func_dec.body};
    this->resolve(s);
    this->end_scope();
}


void Resolver::visit_expr_stmt(ExpressionStatementNode& stmt) {
    this->resolve(*stmt.expr);
}


void Resolver::visit_if_stmt(IfStatementNode& stmt) {
    if (stmt.condition)
        this->resolve(*stmt.condition);
    this->resolve(*stmt.then_branch);
    if (stmt.else_branch)
        this->resolve(*stmt.else_branch);
}


void Resolver::visit_print_stmt(PrintStatementNode& stmt) {
    this->resolve(*stmt.expr);
}


void Resolver::visit_return_stmt(ReturnStatementNode& stmt) {
    if (stmt.expr)
        this->resolve(*stmt.expr);
}


void Resolver::visit_while_stmt(WhileStatementNode& stmt) {
    if (stmt.condition)
        this->resolve(*stmt.condition);
    this->resolve(*stmt.body);
}


void Resolver::visit_bin_expr(BinaryNode& expr) {
    this->resolve(*expr.left);
    this->resolve(*expr.right);
}


void Resolver::visit_call_expr(CallNode& expr) {
    this->resolve(*expr.callee);
    if (expr.args) {
        for (auto v : *expr.args) {
            this->resolve(*v);
        }
    }
}


void Resolver::visit_literal_expr(LiteralNode&) {}


void Resolver::visit_logical_expr(LogicalNode& expr) {
    this->resolve(*expr.left);
    this->resolve(*expr.right);
}


void Resolver::visit_unary_expr(UnaryNode& expr) {
    this->resolve(*expr.operand);
}

