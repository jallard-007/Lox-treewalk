#pragma once

#include <optional>
#include <expected>
#include <span>
#include "node.hpp"
#include "token.hpp"
#include "errors.hpp"
#include "environment.hpp"

struct Interpreter {
    Environment global_env {};
    Environment* environment;

    bool repl_mode = false;

    Interpreter(): environment{&this->global_env} {}
    explicit Interpreter(bool repl_mode): environment{&this->global_env}, repl_mode{repl_mode} {}

    std::optional<InterpreterError> execute(const StatementNode&);
    std::optional<InterpreterError> execute_block(const BlockStatementNode&, Environment&);
    std::expected<Object, InterpreterError> evaluate(const ExpressionNode&);

    std::optional<InterpreterError> visit_statement_node(const StatementNode&);
    std::optional<InterpreterError> visit_print_statement_node(const PrintStatementNode&);
    std::optional<InterpreterError> visit_expression_statement_node(const ExpressionStatementNode&);
    std::optional<InterpreterError> visit_variable_statement_node(const VariableDefStatementNode&);
    std::optional<InterpreterError> visit_block_statement_node(const BlockStatementNode&);

    std::expected<Object, InterpreterError> visit_unary_expr(const UnaryNode&);
    std::expected<Object, InterpreterError> visit_binary_expr(const BinaryNode&);
    std::expected<Object, InterpreterError> visit_variable_expr(const VariableNode&);
    std::expected<Object, InterpreterError> visit_assignment_expr(const AssignmentNode&);

    std::optional<InterpreterError> print_expression(const ExpressionNode&);

    bool is_truthy(const Object&) const;
    bool is_equal(const Object&, const Object&) const;

    void interpret(const std::span<StatementNode*>&);
};
