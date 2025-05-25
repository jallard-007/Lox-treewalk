#pragma once

#include "node.hpp"
#include "interpreter.hpp"


class LoxCallable {
public:
    virtual size_t arity() = 0;
    virtual std::optional<InterpreterSignal> call(Interpreter&, std::vector<Object>&) = 0;
    virtual std::string to_string() = 0;
    virtual ~LoxCallable() = default;
};


class LoxFunction: public LoxCallable {
public:
    const FunctionDeclarationNode* declaration;
    std::shared_ptr<Environment> closure;
    bool is_initializer;
    LoxFunction(const FunctionDeclarationNode& declaration, std::shared_ptr<Environment> closure, bool is_initializer):
        declaration{&declaration}, closure{closure}, is_initializer{is_initializer} {}

    size_t arity() {
        return this->declaration->params->size();
    }

    std::optional<InterpreterSignal> call(Interpreter& interpreter, std::vector<Object>& arguments) {
        auto environment = std::make_shared<Environment>(this->closure);
        for (size_t i = 0; i < this->declaration->params->size(); i++) {
            environment->define(this->declaration->params->at(i)->lexeme, arguments[i]);
        }

        auto ret = interpreter.execute_block(*this->declaration->body, environment);
        if (ret.has_value() && std::holds_alternative<InterpreterError>(ret.value())) {
            return ret.value();
        }
        if (this->is_initializer) {
            auto res = this->closure->get_at(0, 0);
            if (!res.has_value()) {
                return res.error();
            }
            return ReturnSignal{res.value()};
        }
        return ret;
    }

    std::string to_string() {
        return "<fn " + std::string(this->declaration->name->lexeme) + ">";
    }

    std::shared_ptr<LoxFunction> bind(std::shared_ptr<LoxInstance> instance) {
        auto env = std::make_shared<Environment>(this->closure);
        env->define("this", instance);
        return std::make_shared<LoxFunction>(*this->declaration, env, this->is_initializer);
    }

};
