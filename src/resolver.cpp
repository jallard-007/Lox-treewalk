#include "resolver.hpp"
#include "lox.hpp"
#include <stdexcept>

Resolver::Resolver(Interpreter& inter): interpreter{inter} {}


void Resolver::visit_block(BlockStatementNode& block) {
    this->begin_scope();
    this->resolve(*block.stmts);
    this->end_scope();
}


void Resolver::begin_scope() {
    this->scopes.emplace_back();
}

void Resolver::end_scope() {
    auto& s = this->scopes.back();
    for (auto& v : s) {
        if (!v.second.used) {
            Lox::error(*v.second.tk, "Unused variable");
        }
    }
    this->scopes.pop_back();
}


void Resolver::resolve(std::vector<StatementNode*>& stmts) {
    for (auto stmt : stmts) {
        this->resolve(*stmt);
    }
}


void Resolver::resolve(StatementNode& stmt) {
    switch (stmt.get_type()) {
        case StatementType::PRINT: { this->visit_print_stmt(*stmt.get_print_statement_node()); break; }
        case StatementType::EXPRESSION: { this->visit_expr_stmt(*stmt.get_expression_statement_node()); break; }
        case StatementType::VARIABLE: { this->visit_var_dec_node(*stmt.get_variable_statement_node()); break; }
        case StatementType::BLOCK: { this->visit_block(*stmt.get_block_statement_node()); break; }
        case StatementType::IF: { this->visit_if_stmt(*stmt.get_if_statement_node()); break; }
        case StatementType::WHILE: { this->visit_while_stmt(*stmt.get_while_statement_node()); break; }
        case StatementType::BREAK: { this->visit_break_stmt(*stmt.get_break_statement_node()); break; }
        case StatementType::RETURN: { this->visit_return_stmt(*stmt.get_return_statement_node()); break; }
        case StatementType::FUNCTION: { this->visit_function_dec(stmt); break; }
    }
}


void Resolver::resolve(ExpressionNode& expr) {
    switch (expr.get_type()) {
        case ExpressionType::BINARYOP: { this->visit_bin_expr(*expr.get_binary_node()); break;}
        case ExpressionType::UNARYOP: { this->visit_unary_expr(*expr.get_unary_node()); break;}
        case ExpressionType::LITERAL: { this->visit_literal_expr(*expr.get_literal_node()); break;}
        case ExpressionType::VARIABLE: { this->visit_var_expr(expr); break;}
        case ExpressionType::ASSIGNMENT: { this->visit_assign_expr(expr); break;}
        case ExpressionType::LOGICAL: { this->visit_logical_expr(*expr.get_logical_node()); break;}
        case ExpressionType::CALL: { this->visit_call_expr(*expr.get_call_node()); break;}
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
    auto& scope = this->scopes.back();
    bool contains = scope.contains(tk.lexeme);
    auto& v = scope[std::string(tk.lexeme)] = VarInfo{false, false, &tk};
    if (contains) {
        Lox::error(tk, "Already a variable with this name in this scope.");
    } else {
        v.index = scope.size() - 1;
    }
}


void Resolver::define(Token& tk) {
    if (this->scopes.empty()) return;

    auto& s = this->scopes.back();
    auto d = s.find(tk.lexeme);
    if (d == s.end()) {
        throw std::runtime_error("Variable defined but not declared");
    }
    d->second.defined = true;
}


void Resolver::visit_var_expr(ExpressionNode& expr) {
    VariableNode& var_expr = *expr.get_variable_node();
    if (!this->scopes.empty()) {
        auto& s = this->scopes.back();
        auto d = s.find(var_expr.name->lexeme);
        if (d != s.end()) {
            if (d->second.defined == false) {
                Lox::error(*var_expr.name, "Can't read local variable in its own initializer.");
            } else {
                d->second.used = true;
            }
        }
    }

    this->resolve_local(expr, *var_expr.name);
}


void Resolver::resolve_local(ExpressionNode& expr, Token& name) {
    for (int i = this->scopes.size() - 1; i >= 0; i--) {
        if (auto v = this->scopes[i].find(name.lexeme); v != this->scopes[i].end()) {
            this->interpreter.resolve(&expr, this->scopes.size() - 1 - i, v->second.index);
            return;
        }
    }
}

void Resolver::visit_assign_expr(ExpressionNode& expr) {
    AssignmentNode& assign_expr = *expr.get_assignment_node();
    this->resolve(*assign_expr.expr);
    this->resolve_local(expr, *assign_expr.name);
}

void Resolver::visit_function_dec(StatementNode& stmt) {
    FunctionDeclarationNode& func_dec = *stmt.get_function_declaration_node();
    this->declare(*func_dec.name);
    this->define(*func_dec.name);
    this->resolve_function(func_dec, FunctionType::FUNCTION);
}

void Resolver::resolve_function(FunctionDeclarationNode& func_dec, FunctionType type) {
    FunctionType enclosing_func = this->current_function;
    this->current_function = type;
    this->begin_scope();
    if (func_dec.params){
        for (auto& p : *func_dec.params) {
            this->declare(*p);
            this->define(*p);
        }
    }
    this->resolve(*func_dec.body->stmts);
    this->end_scope();
    this->current_function = enclosing_func;
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
    if (this->current_function == FunctionType::NONE) {
        Lox::error(*stmt.rt, "Can't return from top-level code.");
    }
    if (stmt.expr)
        this->resolve(*stmt.expr);
}


void Resolver::visit_break_stmt(BreakStatementNode& br) {
    if (this->loop_depth == 0) {
        Lox::error(*br.tk, "Can't use 'break' outside of loop");
    }
}


void Resolver::visit_while_stmt(WhileStatementNode& stmt) {
    if (stmt.condition)
        this->resolve(*stmt.condition);
    this->loop_depth++;
    this->resolve(*stmt.body);
    this->loop_depth--;
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

