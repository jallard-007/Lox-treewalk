#pragma once

#include <chrono>
#include "interpreter.hpp"
#include "lox_callable.hpp"
#include "token.hpp"

class ClockCallable: public LoxCallable {
public:
    size_t arity() { return 0; }

    std::optional<InterpreterSignal> call(Interpreter&, std::vector<Object>&) {
        auto t = std::chrono::system_clock::now();
        return ReturnSignal(Object{std::chrono::duration_cast<std::chrono::duration<double>>(t.time_since_epoch()).count()});
    }

    std::string to_string() { return "<native fn>"; }
};
