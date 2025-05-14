#pragma once

#include "node.hpp"
#include "interpreter.hpp"


class LoxCallable {
public:
    virtual int arity() = 0;
    virtual std::optional<InterpreterSignal> call(Interpreter&, std::vector<Object>&) = 0;
    virtual ~LoxCallable() = default;
};


class LoxFunction: public LoxCallable {
public:
    const FunctionDeclarationNode* declaration;
    LoxFunction(const FunctionDeclarationNode& declaration): declaration{&declaration} {}

    int arity() {
        return this->declaration->params->size();
    }

    std::optional<InterpreterSignal> call(Interpreter& interpreter, std::vector<Object>& arguments) {
        Environment environment {interpreter.global_env};
        for (int i = 0; i < this->declaration->params->size(); i++) {
            environment.define(this->declaration->params->at(i)->lexeme, arguments[i]);
        }

        return interpreter.execute_block(*this->declaration->body, environment);
    }

    std::string to_string() {
        return "<fn " + std::string(this->declaration->name->lexeme) + ">";
    }
};
