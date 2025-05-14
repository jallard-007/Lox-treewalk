#include "environment.hpp"
#include <iostream>


void Environment::define(std::string_view n, Object v) {
    this->values[std::string(n)] = std::move(v);
}


std::expected<Object, InterpreterError> Environment::get(const Token& name) const {
    auto res = this->values.find(name.lexeme);
    if (res == this->values.end()) {
        if (this->enclosing) {
            return this->enclosing->get(name);
        }
        return std::unexpected(InterpreterError(InterpreterErrorType::UndefinedVariable, name, "Undefined variable '" + std::string(name.lexeme) + "'."));
    }
    return res->second;
}


std::optional<InterpreterError> Environment::assign(const Token& name, Object value) {
    auto res = this->values.find(name.lexeme);

    if (res == this->values.end()) {
        if (this->enclosing) {
            return this->enclosing->assign(name, std::move(value));
        }
        return InterpreterError(InterpreterErrorType::UndefinedVariable, name, "Undefined variable '" + std::string(name.lexeme) + "'.");
    }

    res->second = std::move(value);
    return std::nullopt;
}
