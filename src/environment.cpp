#include "environment.hpp"
#include <iostream>


void Environment::define(std::string_view n, const Object& v) {
    this->values[std::string(n)] = v;
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


std::optional<InterpreterError> Environment::assign(const Token& name, const Object& value) {
    auto res = this->values.find(name.lexeme);

    if (res == this->values.end()) {
        if (this->enclosing) {
            return this->enclosing->assign(name, value);
        }
        return InterpreterError(InterpreterErrorType::UndefinedVariable, name, "Undefined variable '" + std::string(name.lexeme) + "'.");
    }

    res->second = value;
    return std::nullopt;
}
