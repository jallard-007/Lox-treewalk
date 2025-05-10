#include <optional>
#include "interpreter.hpp"
#include "lox.hpp"


std::string stringify(const Object& v) {
    return std::visit([](const auto& vs) -> std::string {
        using T = std::decay_t<decltype(vs)>;

        if constexpr (std::is_same_v<T, None>) {
            return "nil";
        } else if constexpr (std::is_same_v<T, Number>) {
            std::string text = std::to_string(vs);
            if (text.ends_with(".0")) {
                text = text.substr(0, text.length() - 2);
            }
            return text;
        } else if constexpr (std::is_same_v<T, bool>) {
            return vs ? "true" : "false";
        } else if constexpr (std::is_same_v<T, String>) {
            return vs;
        } else {
            return "STRINGIFY ERROR: Invalid Token Value!";
        }
    }, v);
}


std::optional<InterpreterError> check_number_operand(const Token& oper, const Object& operand) {
    if (std::holds_alternative<double>(operand)) return std::nullopt;
    return InterpreterError(InterpreterErrorType::MustBeNumbers, oper, "Operand must be a number.");
}

std::optional<InterpreterError> check_number_operands(const Token& oper, const Object& left, const Object& right) {
    if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) return std::nullopt;
    return InterpreterError(InterpreterErrorType::MustBeNumbers, oper, "Operands must be numbers.");
}


std::optional<InterpreterError> Interpreter::visit_statement_node(const StatementNode& stmt) {
    using enum StatementType;
    switch (stmt.get_type()) {
        case PRINT: return this->visit_print_statement_node(*stmt.get_print_statement_node());
        case EXPRESSION: return this->visit_expression_statement_node(*stmt.get_expression_statement_node());
        case VARIABLE: return this->visit_variable_statement_node(*stmt.get_variable_statement_node());
        case BLOCK: return this->visit_block_statement_node(*stmt.get_block_statement_node());
    }
    return InterpreterError(InterpreterErrorType::Unimplemented, "Statement type not implemented");
}


std::optional<InterpreterError> Interpreter::visit_block_statement_node(const BlockStatementNode& block_stmt) {
    Environment env {this->environment};
    return this->execute_block(block_stmt, env);
}


std::optional<InterpreterError> Interpreter::execute_block(const BlockStatementNode& block_stmt, Environment& env) {
    Environment* enclosing = this->environment;
    this->environment = &env;
    for (const auto& stmt : block_stmt.stmts) {
        auto res = this->execute(*stmt);
        if (res.has_value()) {
            this->environment = enclosing;
            return res;
        }
    }
    this->environment = enclosing;
    return std::nullopt;
}


std::optional<InterpreterError> Interpreter::visit_print_statement_node(const PrintStatementNode& stmt) {
    return this->print_expression(*stmt.expr);
}

std::optional<InterpreterError> Interpreter::print_expression(const ExpressionNode& expr) {
    auto res = this->evaluate(expr);
    if (!res.has_value()) {
        return res.error();
    }
    std::cout << stringify(res.value()) << '\n';
    return std::nullopt;
}


std::optional<InterpreterError> Interpreter::visit_expression_statement_node(const ExpressionStatementNode& stmt) {
    if (auto res = this->evaluate(*stmt.expr); !res.has_value()) {
        return res.error();
    }
    return std::nullopt;
}


std::optional<InterpreterError> Interpreter::visit_variable_statement_node(const VariableDefStatementNode& stmt) {
    Object value = None();
    if (stmt.initializer) {
        auto res = evaluate(*stmt.initializer);
        if (!res.has_value()) {
            return res.error();
        }
        value = res.value();
    }

    this->environment->define(stmt.name.lexeme, value);
    return std::nullopt;
}


std::expected<Object, InterpreterError> Interpreter::visit_unary_expr(const UnaryNode& expr) {
    auto right_exp = this->evaluate(*expr.operand);
    if (!right_exp.has_value()) {
        return right_exp;
    }
    auto right = right_exp.value();
    switch (expr.oper.type) {
        case TokenType::MINUS:
            return -std::get<double>(right);
        case TokenType::BANG:
            return !is_truthy(right);
        default:
            break;
    }
    return std::unexpected(InterpreterError(InterpreterErrorType::Unimplemented, expr.oper, "Unary operator not implemented"));
}


std::expected<Object, InterpreterError> Interpreter::visit_binary_expr(const BinaryNode& expr) {
    auto left_exp = this->evaluate(*expr.left);
    if (!left_exp.has_value()) {
        return left_exp;
    }
    auto right_exp = this->evaluate(*expr.right);
    if (!right_exp.has_value()) {
        return right_exp;
    }
    auto& left = left_exp.value();
    auto& right = right_exp.value();

    switch (expr.oper.type) {
        case TokenType::MINUS: {
            if (auto err = check_number_operands(expr.oper, left, right); err.has_value()) {
                return std::unexpected(err.value());
            }
            return std::get<double>(left) - std::get<double>(right);
        }
    
        case TokenType::PLUS: {
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
                return std::get<double>(left) + std::get<double>(right);
            } 

            if (std::holds_alternative<String>(left) && std::holds_alternative<String>(right)) {
                return std::get<String>(left) + std::get<String>(right);
            }

            return std::unexpected(InterpreterError(InterpreterErrorType::BinOpValuesNotCompatible, expr.oper, "Binary operator values not compatible"));
        }

        case TokenType::SLASH: {
            if (auto err = check_number_operands(expr.oper, left, right); err.has_value()) {
                return std::unexpected(err.value());
            }
            return std::get<double>(left) / std::get<double>(right);
        }
        case TokenType::STAR: {
            if (auto err = check_number_operands(expr.oper, left, right); err.has_value()) {
                return std::unexpected(err.value());
            }
            return std::get<double>(left) * std::get<double>(right);
        }
        case TokenType::GREATER: {
            if (auto err = check_number_operands(expr.oper, left, right); err.has_value()) {
                return std::unexpected(err.value());
            }
            return std::get<double>(left) > std::get<double>(right);
        }
        case TokenType::GREATER_EQUAL: {
            if (auto err = check_number_operands(expr.oper, left, right); err.has_value()) {
                return std::unexpected(err.value());
            }
            return std::get<double>(left) >= std::get<double>(right);
        }
        case TokenType::LESS: {
            if (auto err = check_number_operands(expr.oper, left, right); err.has_value()) {
                return std::unexpected(err.value());
            }
            return std::get<double>(left) < std::get<double>(right);
        }
        case TokenType::LESS_EQUAL: {
            if (auto err = check_number_operands(expr.oper, left, right); err.has_value()) {
                return std::unexpected(err.value());
            }
            return std::get<double>(left) <= std::get<double>(right);
        }

        case TokenType::BANG_EQUAL: return !this->is_equal(left, right);
        case TokenType::EQUAL_EQUAL: return this->is_equal(left, right);

        default:
            break;
    }

    return std::unexpected(InterpreterError(InterpreterErrorType::Unimplemented, expr.oper, "Binary operator not implemented"));
}


std::expected<Object, InterpreterError> Interpreter::visit_variable_expr(const VariableNode& expr) {
    return this->environment->get(expr.name);
}


std::expected<Object, InterpreterError> Interpreter::visit_assignment_expr(const AssignmentNode& expr) {
    auto value = this->evaluate(*expr.expr);
    if (!value.has_value()) {
        return value;
    }
    if (auto err = this->environment->assign(expr.name, value.value()); err.has_value()) {
        return std::unexpected(err.value());
    }
    return value;
}


std::expected<Object, InterpreterError> Interpreter::evaluate(const ExpressionNode& expr) {
    using enum ExpressionType;
    switch (expr.get_type()) {
        case LITERAL: return expr.get_literal_node()->value;
        case BINARYOP: return this->visit_binary_expr(*expr.get_binary_node());
        case UNARYOP: return this->visit_unary_expr(*expr.get_unary_node());
        case VARIABLE: return this->visit_variable_expr(*expr.get_variable_node());
        case ASSIGNMENT: return this->visit_assignment_expr(*expr.get_assignment_node());
    }
    

    return std::unexpected(InterpreterError(InterpreterErrorType::Unimplemented, "Expression type not implemented"));
}


bool Interpreter::is_truthy(const Object& v) const {
    if (std::holds_alternative<None>(v)) return false;
    if (std::holds_alternative<bool>(v)) return std::get<bool>(v);
    return true;
}


bool Interpreter::is_equal(const Object& a, const Object& b) const {
    return std::visit([](const auto& lhs, const auto& rhs) -> bool {
        using L = std::decay_t<decltype(lhs)>;
        using R = std::decay_t<decltype(rhs)>;
        if constexpr (std::is_same_v<L, R>) {
            return lhs == rhs;
        } else {
            return false;
        }
    }, a, b);
}


void Interpreter::interpret(const std::span<StatementNode*>& stmts) {
    for (const auto& stmt : stmts) {
        if (repl_mode && stmt->get_type() == StatementType::EXPRESSION) {
            if (auto res = this->print_expression(*stmt->get_expression_statement_node()->expr); res.has_value()) {
                Lox::runtime_error(res.value());
            }
        } else {
            auto res = this->execute(*stmt);
            if (res.has_value()) {
                Lox::runtime_error(res.value());
            }
        }
    }
}


std::optional<InterpreterError> Interpreter::execute(const StatementNode& stmt) {
    return this->visit_statement_node(stmt);
}
