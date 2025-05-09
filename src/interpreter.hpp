#pragma once

#include <expected>
#include <vector>
#include <string>
#include "node.hpp"
#include "token.hpp"
#include "errors.hpp"
#include "environment.hpp"

struct Interpreter {
    Environment environment;

    std::optional<InterpreterError> execute(const StatementNode&);
    std::expected<Object, InterpreterError> evaluate(const ExpressionNode&);

    std::optional<InterpreterError> visit_statement_node(const StatementNode&);
    std::optional<InterpreterError> visit_print_statement_node(const PrintStatementNode&);
    std::optional<InterpreterError> visit_expression_statement_node(const ExpressionStatementNode&);
    std::optional<InterpreterError> visit_variable_statement_node(const VariableDefStatementNode&);

    std::expected<Object, InterpreterError> visit_unary_expr(const UnaryNode&);
    std::expected<Object, InterpreterError> visit_binary_expr(const BinaryNode&);
    std::expected<Object, InterpreterError> visit_variable_expr(const VariableNode&);
    std::expected<Object, InterpreterError> visit_assignment_expr(const AssignmentNode&);


    bool is_truthy(const Object&);
    bool is_equal(const Object&, const Object&);

    void interpret(const std::vector<StatementNode*>&);
};
