#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include "interpreter.hpp"
#include "node.hpp"
#include "string_hash.hpp"

struct Resolver {
    Interpreter& interpreter;
    std::vector<std::unordered_map<std::string, bool, string_hash, std::equal_to<>>> scopes;
    Resolver(Interpreter&);

    void block(BlockStatementNode&);
    void begin_scope();
    void end_scope();
    void resolve(std::vector<StatementNode*>&);
    void resolve(StatementNode&);
    void resolve(ExpressionNode&);

    void visit_var_dec_node(VariableDeclarationNode&);
    void visit_assign_expr(AssignmentNode&);
    void visit_function_dec(FunctionDeclarationNode&);

    void visit_expr_stmt(ExpressionStatementNode&);
    void visit_if_stmt(IfStatementNode&);
    void visit_print_stmt(PrintStatementNode&);
    void visit_return_stmt(ReturnStatementNode&);
    void visit_while_stmt(WhileStatementNode&);
    void visit_bin_expr(BinaryNode&);
    void visit_call_expr(CallNode&);
    void visit_literal_expr(LiteralNode&);
    void visit_logical_expr(LogicalNode&);
    void visit_unary_expr(UnaryNode&);

    void declare(Token&);
    void define(Token&);

    void visit_var_expr(VariableNode&);

    void resolve_local(ExpressionNode& expr, Token& name);
    void resolve_function(FunctionDeclarationNode&);
};
