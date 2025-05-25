#pragma once

#include <optional>
#include <expected>
#include <span>
#include "node.hpp"
#include "token.hpp"
#include "errors.hpp"
#include "environment.hpp"

struct BreakSignal {};
struct ReturnSignal {
    Object value;
};

using InterpreterSignal = std::variant<InterpreterError, BreakSignal, ReturnSignal>;

struct Interpreter {
    std::shared_ptr<Environment> global_env;
    std::shared_ptr<Environment> environment;

    bool repl_mode = false;

    Interpreter();
    explicit Interpreter(bool);

    [[nodiscard]] std::optional<InterpreterSignal> execute(const StatementNode&);
    [[nodiscard]] std::optional<InterpreterSignal> execute_block(const BlockStatementNode&, std::shared_ptr<Environment>);
    [[nodiscard]] std::expected<Object, InterpreterSignal> evaluate(const ExpressionNode&);

    [[nodiscard]] std::optional<InterpreterSignal> visit_statement_node(const StatementNode&);
    [[nodiscard]] std::optional<InterpreterSignal> visit_print_statement_node(const PrintStatementNode&);
    [[nodiscard]] std::optional<InterpreterSignal> visit_expression_statement_node(const ExpressionStatementNode&);
    [[nodiscard]] std::optional<InterpreterSignal> visit_variable_declaration_node(const VariableDeclarationNode&);
    [[nodiscard]] std::optional<InterpreterSignal> visit_block_statement_node(const BlockStatementNode&);
    [[nodiscard]] std::optional<InterpreterSignal> visit_if_statement_node(const IfStatementNode&);
    [[nodiscard]] std::optional<InterpreterSignal> visit_while_statement_node(const WhileStatementNode&);
    [[nodiscard]] BreakSignal visit_break_statement_node(const BreakStatementNode&) const;
    [[nodiscard]] InterpreterSignal visit_return_statement_node(const ReturnStatementNode&);
    [[nodiscard]] std::optional<InterpreterSignal> visit_function_declaration_node(const FunctionDeclarationNode&);

    [[nodiscard]] std::expected<Object, InterpreterSignal> visit_unary_expr(const UnaryNode&);
    [[nodiscard]] std::expected<Object, InterpreterSignal> visit_binary_expr(const BinaryNode&);
    [[nodiscard]] std::expected<Object, InterpreterSignal> visit_variable_expr(const VariableNode&);
    [[nodiscard]] std::expected<Object, InterpreterSignal> visit_assignment_expr(const AssignmentNode&);
    [[nodiscard]] std::expected<Object, InterpreterSignal> visit_logical_expr(const LogicalNode&);
    [[nodiscard]] std::expected<Object, InterpreterSignal> visit_call_expr(const CallNode&);

    [[nodiscard]] std::optional<InterpreterSignal> print_expression(const ExpressionNode&);

    bool is_truthy(const Object&) const;
    bool is_equal(const Object&, const Object&) const;

    void interpret(const std::span<StatementNode*>&);

    void resolve(const ExpressionNode&, int);

};
