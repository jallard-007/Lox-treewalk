#pragma once

#include <string>
#include "token.hpp"


enum class InterpreterErrorType {
    Unimplemented,
    BinOpValuesNotCompatible,
    MustBeNumbers,
    UndefinedVariable
};

struct InterpreterError {
    const InterpreterErrorType type;
    const Token where;
    const std::string msg;
    InterpreterError(InterpreterErrorType t, Token where, std::string msg): type{t}, where{std::move(where)}, msg{std::move(msg)} {}
    InterpreterError(InterpreterErrorType t, std::string msg): type{t}, where{}, msg{std::move(msg)} {}
};
