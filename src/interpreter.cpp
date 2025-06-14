#include <optional>
#include "interpreter.hpp"
#include "lox_callable.hpp"
#include "lox_class.hpp"
#include "lox_instance.hpp"
#include "lox_builtins.hpp"
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
            return *vs;
        } else if constexpr (std::is_same_v<T, std::shared_ptr<LoxCallable>>) {
            return vs->to_string();
        } else if constexpr (std::is_same_v<T, std::shared_ptr<LoxInstance>>) {
            return vs->to_string();
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


Interpreter::Interpreter(): Interpreter(false) {}
Interpreter::Interpreter(bool repl_mode): repl_mode{repl_mode} {
    this->global_env = std::make_shared<Environment>();
    this->global_env->define("clock", std::make_shared<ClockCallable>());
    this->environment = this->global_env;
}

std::optional<InterpreterSignal> Interpreter::visit_statement_node(const StatementNode& stmt) {
    using enum StatementType;
    switch (stmt.get_type()) {
        case PRINT: return this->visit_print_statement_node(*stmt.get_print_statement_node());
        case EXPRESSION: return this->visit_expression_statement_node(*stmt.get_expression_statement_node());
        case VARIABLE: return this->visit_variable_declaration_node(*stmt.get_variable_statement_node());
        case BLOCK: return this->visit_block_statement_node(*stmt.get_block_statement_node());
        case IF: return this->visit_if_statement_node(*stmt.get_if_statement_node());
        case WHILE: return this->visit_while_statement_node(*stmt.get_while_statement_node());
        case BREAK: return this->visit_break_statement_node(*stmt.get_break_statement_node());
        case RETURN: return this->visit_return_statement_node(*stmt.get_return_statement_node());
        case FUNCTION: return this->visit_function_declaration_node(*stmt.get_function_declaration_node());
        case CLASS: return this->visit_class_declaration_node(*stmt.get_class_declaration_node());
    }
    return InterpreterError(InterpreterErrorType::Unimplemented, "Statement type not implemented");
}


std::optional<InterpreterSignal> Interpreter::visit_block_statement_node(const BlockStatementNode& block_stmt) {
    auto env = std::make_shared<Environment>(this->environment);
    return this->execute_block(block_stmt, env);
}


std::optional<InterpreterSignal> Interpreter::execute_block(const BlockStatementNode& block_stmt, std::shared_ptr<Environment> env) {
    std::shared_ptr<Environment> enclosing = this->environment;
    this->environment = env;
    for (const auto& stmt : *block_stmt.stmts) {
        auto res = this->execute(*stmt);
        if (res.has_value()) {
            this->environment = enclosing;
            return res;
        }
    }
    this->environment = enclosing;
    return std::nullopt;
}


std::optional<InterpreterSignal> Interpreter::visit_print_statement_node(const PrintStatementNode& stmt) {
    return this->print_expression(*stmt.expr);
}

std::optional<InterpreterSignal> Interpreter::print_expression(const ExpressionNode& expr) {
    auto res = this->evaluate(expr);
    if (!res.has_value()) {
        return res.error();
    }
    std::cout << stringify(res.value()) << '\n';
    return std::nullopt;
}


std::optional<InterpreterSignal> Interpreter::visit_expression_statement_node(const ExpressionStatementNode& stmt) {
    if (auto res = this->evaluate(*stmt.expr); !res.has_value()) {
        return res.error();
    }
    return std::nullopt;
}


std::optional<InterpreterSignal> Interpreter::visit_variable_declaration_node(const VariableDeclarationNode& stmt) {
    Object value = None();
    if (stmt.initializer) {
        auto res = evaluate(*stmt.initializer);
        if (!res.has_value()) {
            return res.error();
        }
        value = res.value();
    }
    this->environment->define(stmt.name->lexeme, value);
    return std::nullopt;
}


std::optional<InterpreterSignal> Interpreter::visit_if_statement_node(const IfStatementNode& stmt) {
    auto condition = this->evaluate(*stmt.condition);
    if (!condition.has_value()) {
        return condition.error();
    }
    if (this->is_truthy(condition.value())) {
        return this->visit_statement_node(*stmt.then_branch);
    }
    if (stmt.else_branch) {
        return this->visit_statement_node(*stmt.else_branch);
    }

    return std::nullopt;
}


std::optional<InterpreterSignal> Interpreter::visit_while_statement_node(const WhileStatementNode& stmt) {
    while (true) {
        {
            auto res = this->evaluate(*stmt.condition);
            if (!res.has_value()) {
                return res.error();
            }
            if (!this->is_truthy(res.value())) {
                return std::nullopt;
            }
        }
        if (auto res = this->visit_statement_node(*stmt.body); res.has_value()) {
            if (std::holds_alternative<BreakSignal>(res.value())) {
                return std::nullopt;
            }
            return res;
        }
    } 
}


BreakSignal Interpreter::visit_break_statement_node(const BreakStatementNode&) const {
    return BreakSignal{};
}

InterpreterSignal Interpreter::visit_return_statement_node(const ReturnStatementNode& stmt) {
    if (!stmt.expr) {
        return ReturnSignal{None()};
    }
    auto res = this->evaluate(*stmt.expr);
    if (!res.has_value()) {
        return res.error();
    }
    return ReturnSignal{res.value()};
}

std::optional<InterpreterSignal> Interpreter::visit_function_declaration_node(const FunctionDeclarationNode& func) {
    this->environment->define(func.name->lexeme, None());
    this->environment->assign(*func.name, std::make_shared<LoxFunction>(func, this->environment, false));
    return std::nullopt;
}


std::optional<InterpreterSignal> Interpreter::visit_class_declaration_node(const ClassDeclarationNode& class_) {
    this->environment->define(class_.name->lexeme, None());
    std::unordered_map<std::string, std::shared_ptr<LoxFunction>, string_hash, std::equal_to<>> methods;
    for (auto& method : *class_.methods) {
        methods[std::string(method->name->lexeme)] = std::make_shared<LoxFunction>(*method, this->environment, method->name->lexeme == "init");
    }
    this->environment->assign(*class_.name, std::make_shared<LoxClass>(class_.name->lexeme, std::move(methods)));
    return std::nullopt;
}

std::expected<Object, InterpreterSignal> Interpreter::visit_unary_expr(const UnaryNode& expr) {
    auto right_exp = this->evaluate(*expr.operand);
    if (!right_exp.has_value()) {
        return right_exp;
    }
    auto right = right_exp.value();
    switch (expr.oper->type) {
        case TokenType::MINUS:
            return -std::get<double>(right);
        case TokenType::BANG:
            return !is_truthy(right);
        default:
            break;
    }
    return std::unexpected(InterpreterError(InterpreterErrorType::Unimplemented, *expr.oper, "Unary operator not implemented"));
}


std::expected<Object, InterpreterSignal> Interpreter::visit_binary_expr(const BinaryNode& expr) {
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

    switch (expr.oper->type) {
        case TokenType::MINUS: {
            if (auto err = check_number_operands(*expr.oper, left, right); err.has_value()) {
                return std::unexpected(err.value());
            }
            return std::get<double>(left) - std::get<double>(right);
        }
    
        case TokenType::PLUS: {
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
                return std::get<double>(left) + std::get<double>(right);
            } 

            if (std::holds_alternative<String>(left) && std::holds_alternative<String>(right)) {
                return std::make_shared<std::string>(*std::get<String>(left) + *std::get<String>(right));
            }

            return std::unexpected(InterpreterError(InterpreterErrorType::BinOpValuesNotCompatible, *expr.oper, "Binary operator values not compatible"));
        }

        case TokenType::SLASH: {
            if (auto err = check_number_operands(*expr.oper, left, right); err.has_value()) {
                return std::unexpected(err.value());
            }
            return std::get<double>(left) / std::get<double>(right);
        }
        case TokenType::STAR: {
            if (auto err = check_number_operands(*expr.oper, left, right); err.has_value()) {
                return std::unexpected(err.value());
            }
            return std::get<double>(left) * std::get<double>(right);
        }
        case TokenType::GREATER: {
            if (auto err = check_number_operands(*expr.oper, left, right); err.has_value()) {
                return std::unexpected(err.value());
            }
            return std::get<double>(left) > std::get<double>(right);
        }
        case TokenType::GREATER_EQUAL: {
            if (auto err = check_number_operands(*expr.oper, left, right); err.has_value()) {
                return std::unexpected(err.value());
            }
            return std::get<double>(left) >= std::get<double>(right);
        }
        case TokenType::LESS: {
            if (auto err = check_number_operands(*expr.oper, left, right); err.has_value()) {
                return std::unexpected(err.value());
            }
            return std::get<double>(left) < std::get<double>(right);
        }
        case TokenType::LESS_EQUAL: {
            if (auto err = check_number_operands(*expr.oper, left, right); err.has_value()) {
                return std::unexpected(err.value());
            }
            return std::get<double>(left) <= std::get<double>(right);
        }

        case TokenType::BANG_EQUAL: return !this->is_equal(left, right);
        case TokenType::EQUAL_EQUAL: return this->is_equal(left, right);

        default:
            break;
    }

    return std::unexpected(InterpreterError(InterpreterErrorType::Unimplemented, *expr.oper, "Binary operator not implemented"));
}


std::expected<Object, InterpreterSignal> Interpreter::visit_variable_expr(const ExpressionNode& expr) {
    VariableNode& var_expr = *expr.get_variable_node();
    return this->look_up_variable(*var_expr.name, &expr);
    // return this->environment->get(*expr.name);
}


std::expected<Object, InterpreterSignal> Interpreter::visit_assignment_expr(const ExpressionNode& expr) {
    AssignmentNode& assign_node = *expr.get_assignment_node();
    auto value = this->evaluate(*assign_node.expr);
    if (!value.has_value()) {
        return value;
    }
    if (auto l = this->locals.find(&expr); l != this->locals.end()) {
        if (auto err = this->environment->assign_at(l->second.depth, l->second.index, value.value()); err.has_value()) {
            return std::unexpected(err.value());
        }
    } else {
        if (auto err = this->global_env->assign(*assign_node.name, value.value()); err.has_value()) {
            return std::unexpected(err.value());
        }
    }

    return value;
}


std::expected<Object, InterpreterSignal> Interpreter::visit_logical_expr(const LogicalNode& expr) {
    auto left = this->evaluate(*expr.left);

    if (expr.oper->type == TokenType::OR) {
      if (this->is_truthy(left.value())) return left;
    } else {
      if (!this->is_truthy(left.value())) return left;
    }
    return evaluate(*expr.right);
}


std::expected<Object, InterpreterSignal> Interpreter::visit_call_expr(const CallNode& expr) {
    auto callee = this->evaluate(*expr.callee);
    if (!callee.has_value()) {
        return callee;
    }

    std::vector<Object> arguments;
    if (expr.args) {
        for (const ExpressionNode* argument : *expr.args) {
            auto res = evaluate(*argument);
            if (!res.has_value()) {
                return res;
            }
            arguments.push_back(res.value());
        }
    }

    auto function = std::get_if<std::shared_ptr<LoxCallable>>(&callee.value());
    if (!function) {
        return std::unexpected(InterpreterError(InterpreterErrorType::NotCallable, *expr.paren, "Can only call functions and classes"));
    }
    if (arguments.size() != (*function)->arity()) {
        return std::unexpected(InterpreterError(InterpreterErrorType::Arity, *expr.paren,
            std::format("Expected {} arguments but got {}.", (*function)->arity(), arguments.size())
        ));
    }
    auto res = (*function)->call(*this, arguments);
    if (res.has_value()) {
        if (std::holds_alternative<ReturnSignal>(res.value())) {
            return std::get<ReturnSignal>(res.value()).value;
        }
        return std::unexpected(res.value());
    }
    return None();
}


std::expected<Object, InterpreterSignal> Interpreter::visit_get_expr(const GetNode& expr) {
    auto obj = this->evaluate(*expr.object);
    if (!obj.has_value()) {
        return std::unexpected(obj.error());
    }
    if (!std::holds_alternative<std::shared_ptr<LoxInstance>>(obj.value())) {
        return std::unexpected(InterpreterError{InterpreterErrorType::NotInstance, *expr.name, "Only instances have properties"});
    }
    return std::get<std::shared_ptr<LoxInstance>>(obj.value())->get(*expr.name);
}

std::expected<Object, InterpreterSignal> Interpreter::visit_set_expr(const SetNode& expr) {
    auto obj = this->evaluate(*expr.object);
    if (!obj.has_value()) {
        return obj;
    }
    if (!std::holds_alternative<std::shared_ptr<LoxInstance>>(obj.value())) {
        return std::unexpected(InterpreterError{InterpreterErrorType::NotInstance, *expr.name, "Only instances have fields"});
    }
    auto val = this->evaluate(*expr.value);
    if (!val.has_value()) {
        return val;
    }
    std::get<std::shared_ptr<LoxInstance>>(obj.value())->set(*expr.name, val.value()); 
    return val.value();
}

std::expected<Object, InterpreterSignal> Interpreter::visit_this_expr(const ExpressionNode& expr) {
    return this->look_up_variable(*expr.get_this_node()->tk, &expr);
}


std::expected<Object, InterpreterSignal> Interpreter::evaluate(const ExpressionNode& expr) {
    using enum ExpressionType;
    switch (expr.get_type()) {
        case LITERAL: return expr.get_literal_node()->value;
        case BINARYOP: return this->visit_binary_expr(*expr.get_binary_node());
        case UNARYOP: return this->visit_unary_expr(*expr.get_unary_node());
        case VARIABLE: return this->visit_variable_expr(expr);
        case ASSIGNMENT: return this->visit_assignment_expr(expr);
        case LOGICAL: return this->visit_logical_expr(*expr.get_logical_node());
        case CALL: return this->visit_call_expr(*expr.get_call_node());
        case GET: return this->visit_get_expr(*expr.get_get_node());
        case SET: return this->visit_set_expr(*expr.get_set_node());
        case THIS: return this->visit_this_expr(expr);
    }

    return std::unexpected(InterpreterError(InterpreterErrorType::Unimplemented, "Expression type not implemented"));
}


bool Interpreter::is_truthy(const Object& v) const {
    if (std::holds_alternative<None>(v)) return false;
    if (std::holds_alternative<bool>(v)) return std::get<bool>(v);
    if (std::holds_alternative<Number>(v)) return bool(std::get<Number>(v));
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
                std::visit([](const auto& v) {
                    using L = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<L, InterpreterError>) {
                        Lox::runtime_error(v);
                    }
                }, res.value());
            }
            continue;
        }
        auto res = this->execute(*stmt);
        if (res.has_value()) {
            std::visit([](const auto& v) {
                using L = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<L, InterpreterError>) {
                    Lox::runtime_error(v);
                }
            }, res.value());
        }
    }
}


std::optional<InterpreterSignal> Interpreter::execute(const StatementNode& stmt) {
    return this->visit_statement_node(stmt);
}

void Interpreter::resolve(ExpressionNode* expr, int depth, int index) {
    this->locals[expr] = LocalInfo{depth, index};
}

std::expected<Object, InterpreterSignal> Interpreter::look_up_variable(Token& tk, const ExpressionNode * expr) {
    if (auto l = this->locals.find(expr); l != this->locals.end()) {
        return this->environment->get_at(l->second.depth, l->second.index);
    }
    return this->global_env->get(tk);
}
