#pragma once

#include <expected>
#include <vector>
#include <string>
#include "node.hpp"
#include "token.hpp"

enum class InterpreterErrorType {
    Unimplemented,
    BinOpValuesNotCompatible,
    MustBeNumbers,
};

struct InterpreterError {
    const InterpreterErrorType type;
    const Token where;
    const std::string msg;
    InterpreterError(InterpreterErrorType t, Token where, std::string msg): type{t}, where{where}, msg{std::move(msg)} {}
    InterpreterError(InterpreterErrorType t, std::string msg): type{t}, where{}, msg{std::move(msg)} {}
};

struct Interpreter {
    std::optional<InterpreterError> execute(const StatementNode&);
    std::expected<Object, InterpreterError> evaluate(const ExpressionNode&);

    std::optional<InterpreterError> visit_statement_node(const StatementNode&);
    std::optional<InterpreterError> visit_print_statement_node(const PrintStatementNode&);
    std::optional<InterpreterError> visit_expression_statement_node(const ExpressionStatementNode&);

    std::expected<Object, InterpreterError> visit_unary_expr(const UnaryNode&);
    std::expected<Object, InterpreterError> visit_binary_expr(const BinaryNode&);

    bool is_truthy(const Object&);
    bool is_equal(const Object&, const Object&);

    void interpret(const std::vector<StatementNode*>&);
};
