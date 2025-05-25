#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <string_view>
#include "lox_callable.hpp"
#include "string_hash.hpp"

struct LoxInstance;

struct LoxClass: public LoxCallable {
    std::string_view name;
    std::unordered_map<std::string, std::shared_ptr<LoxFunction>, string_hash, std::equal_to<>> methods;

    LoxClass(std::string_view, std::unordered_map<std::string, std::shared_ptr<LoxFunction>, string_hash, std::equal_to<>>);

    std::string to_string();

    std::optional<InterpreterSignal> call(Interpreter&, std::vector<Object>&);

    size_t arity();

    std::shared_ptr<LoxFunction> find_method(std::string_view);
};
