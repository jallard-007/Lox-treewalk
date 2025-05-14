#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>
#include "token.hpp"
#include "tagged_ptr.hpp"
#include "allocator.hpp"

constexpr uint64_t EXPRESSION_NODE_ALIGNMENT_REQ = 8;

struct ExpressionNode;

struct alignas(EXPRESSION_NODE_ALIGNMENT_REQ) LiteralNode {
    Object value;
};


struct alignas(EXPRESSION_NODE_ALIGNMENT_REQ) UnaryNode {
    Token* oper;
    ExpressionNode* operand {};
};


struct alignas(EXPRESSION_NODE_ALIGNMENT_REQ) BinaryNode {
    Token* oper;
    ExpressionNode* left {};
    ExpressionNode* right {};
};


struct alignas(EXPRESSION_NODE_ALIGNMENT_REQ) VariableNode {
    Token* name;
};


struct alignas(EXPRESSION_NODE_ALIGNMENT_REQ) AssignmentNode {
    Token* name;
    ExpressionNode* expr {};
};


struct alignas(EXPRESSION_NODE_ALIGNMENT_REQ) LogicalNode {
    Token* oper;
    ExpressionNode* left {};
    ExpressionNode* right {};
};


struct alignas(EXPRESSION_NODE_ALIGNMENT_REQ) CallNode {
    ExpressionNode* callee {};
    Token* paren;
    std::vector<ExpressionNode*>* args {};
};


constexpr uint8_t expression_mask = 0b111;
enum class ExpressionType : uint8_t {
    BINARYOP,
    UNARYOP,
    LITERAL,
    VARIABLE,
    ASSIGNMENT,
    LOGICAL,
    CALL,

    _LAST = CALL
};
static_assert(std::to_underlying(ExpressionType::_LAST) <= expression_mask);


class ExpressionNode {
    TaggedPtr<void, ExpressionType, expression_mask> tagged;

public:
    explicit ExpressionNode(BinaryNode* v) { this->set_<BinaryNode>(v); }
    explicit ExpressionNode(UnaryNode* v) { this->set_<UnaryNode>(v); }
    explicit ExpressionNode(LiteralNode* v) { this->set_<LiteralNode>(v); }
    explicit ExpressionNode(VariableNode* v) { this->set_<VariableNode>(v); }
    explicit ExpressionNode(AssignmentNode* v) { this->set_<AssignmentNode>(v); }
    explicit ExpressionNode(LogicalNode* v) { this->set_<LogicalNode>(v); }
    explicit ExpressionNode(CallNode* v) { this->set_<CallNode>(v); }

    ExpressionType get_type() const { return tagged.get_tag(); }

    BinaryNode* get_binary_node() const { return this->get<BinaryNode>(); }
    UnaryNode* get_unary_node() const { return this->get<UnaryNode>(); }
    LiteralNode* get_literal_node() const { return this->get<LiteralNode>(); }
    VariableNode* get_variable_node() const { return this->get<VariableNode>(); }
    AssignmentNode* get_assignment_node() const { return this->get<AssignmentNode>(); }
    LogicalNode* get_logical_node() const { return this->get<LogicalNode>(); }
    CallNode* get_call_node() const { return this->get<CallNode>(); }

    void set(BinaryNode* v) { return this->set_<BinaryNode>(v); }
    void set(UnaryNode* v) { return this->set_<UnaryNode>(v); }
    void set(LiteralNode* v) { return this->set_<LiteralNode>(v); }
    void set(VariableNode* v) { return this->set_<VariableNode>(v); }
    void set(AssignmentNode* v) { return this->set_<AssignmentNode>(v); }
    void set(LogicalNode* v) { return this->set_<LogicalNode>(v); }
    void set(CallNode* v) { return this->set_<CallNode>(v); }

private:
    template<typename T>
    static constexpr ExpressionType get_type_for();

    template<typename T>
    void set_(T* ptr) { tagged.set(ptr, get_type_for<T>()); }

    template<typename T>
    T* get() const {
        assert(get_type_for<T>() == get_type());
        return static_cast<T*>(tagged.get_ptr());
    }
};


template<> constexpr ExpressionType ExpressionNode::get_type_for<BinaryNode>() { return ExpressionType::BINARYOP; }
template<> constexpr ExpressionType ExpressionNode::get_type_for<UnaryNode>() { return ExpressionType::UNARYOP; }
template<> constexpr ExpressionType ExpressionNode::get_type_for<LiteralNode>() { return ExpressionType::LITERAL; }
template<> constexpr ExpressionType ExpressionNode::get_type_for<VariableNode>() { return ExpressionType::VARIABLE; }
template<> constexpr ExpressionType ExpressionNode::get_type_for<AssignmentNode>() { return ExpressionType::ASSIGNMENT; }
template<> constexpr ExpressionType ExpressionNode::get_type_for<LogicalNode>() { return ExpressionType::LOGICAL; }
template<> constexpr ExpressionType ExpressionNode::get_type_for<CallNode>() { return ExpressionType::CALL; }


constexpr uint64_t STATEMENT_NODE_ALIGNMENT_REQ = 16;

class StatementNode;

struct alignas(STATEMENT_NODE_ALIGNMENT_REQ) PrintStatementNode {
    ExpressionNode* expr {};
};


struct alignas(STATEMENT_NODE_ALIGNMENT_REQ) ExpressionStatementNode {
    ExpressionNode* expr {};
};


struct alignas(STATEMENT_NODE_ALIGNMENT_REQ) VariableDeclarationNode {
    Token* name;
    ExpressionNode* initializer {};
};


struct alignas(STATEMENT_NODE_ALIGNMENT_REQ) BlockStatementNode {
    std::vector<StatementNode*>* stmts;

};


struct alignas(STATEMENT_NODE_ALIGNMENT_REQ) IfStatementNode {
    ExpressionNode* condition {};
    StatementNode* then_branch {};
    StatementNode* else_branch {};
};


struct alignas(STATEMENT_NODE_ALIGNMENT_REQ) WhileStatementNode {
    ExpressionNode* condition {};
    StatementNode* body {};
};


struct alignas(STATEMENT_NODE_ALIGNMENT_REQ) BreakStatementNode {
};


struct alignas(STATEMENT_NODE_ALIGNMENT_REQ) ReturnStatementNode {
    Token* rt;
    ExpressionNode* expr;
};


struct alignas(STATEMENT_NODE_ALIGNMENT_REQ) FunctionDeclarationNode {
    Token* name;
    std::vector<Token*>* params;
    BlockStatementNode* body;
};


constexpr uint8_t statement_mask = 0b1111;
enum class StatementType {
    PRINT,
    EXPRESSION,
    VARIABLE,
    BLOCK,
    IF,
    WHILE,
    BREAK,
    RETURN,
    FUNCTION,

    _LAST = FUNCTION
};
static_assert(std::to_underlying(StatementType::_LAST) <= statement_mask);



class StatementNode {
    TaggedPtr<void, StatementType, statement_mask> tagged;

public:
    explicit StatementNode(PrintStatementNode* v) { this->set_<PrintStatementNode>(v); }
    explicit StatementNode(ExpressionStatementNode* v) { this->set_<ExpressionStatementNode>(v); }
    explicit StatementNode(VariableDeclarationNode* v) { this->set_<VariableDeclarationNode>(v); }
    explicit StatementNode(BlockStatementNode* v) { this->set_<BlockStatementNode>(v); }
    explicit StatementNode(IfStatementNode* v) { this->set_<IfStatementNode>(v); }
    explicit StatementNode(WhileStatementNode* v) { this->set_<WhileStatementNode>(v); }
    explicit StatementNode(BreakStatementNode* v) { this->set_<BreakStatementNode>(v); }
    explicit StatementNode(ReturnStatementNode* v) { this->set_<ReturnStatementNode>(v); }
    explicit StatementNode(FunctionDeclarationNode* v) { this->set_<FunctionDeclarationNode>(v); }

    StatementType get_type() const { return tagged.get_tag(); }

    PrintStatementNode* get_print_statement_node() const { return this->get<PrintStatementNode>(); }
    ExpressionStatementNode* get_expression_statement_node() const { return this->get<ExpressionStatementNode>(); }
    VariableDeclarationNode* get_variable_statement_node() const { return this->get<VariableDeclarationNode>(); }
    BlockStatementNode* get_block_statement_node() const { return this->get<BlockStatementNode>(); }
    IfStatementNode* get_if_statement_node() const { return this->get<IfStatementNode>(); }
    WhileStatementNode* get_while_statement_node() const { return this->get<WhileStatementNode>(); }
    BreakStatementNode* get_break_statement_node() const { return this->get<BreakStatementNode>(); }
    ReturnStatementNode* get_return_statement_node() const { return this->get<ReturnStatementNode>(); }
    FunctionDeclarationNode* get_function_declaration_node() const { return this->get<FunctionDeclarationNode>(); }

    void set(PrintStatementNode* v) { return this->set_<PrintStatementNode>(v); }
    void set(ExpressionStatementNode* v) { return this->set_<ExpressionStatementNode>(v); }
    void set(VariableDeclarationNode* v) { return this->set_<VariableDeclarationNode>(v); }
    void set(BlockStatementNode* v) { return this->set_<BlockStatementNode>(v); }
    void set(IfStatementNode* v) { return this->set_<IfStatementNode>(v); }
    void set(WhileStatementNode* v) { return this->set_<WhileStatementNode>(v); }
    void set(BreakStatementNode* v) { return this->set_<BreakStatementNode>(v); }
    void set(ReturnStatementNode* v) { return this->set_<ReturnStatementNode>(v); }
    void set(FunctionDeclarationNode* v) { return this->set_<FunctionDeclarationNode>(v); }

private:
    template<typename T>
    static constexpr StatementType get_type_for();

    template<typename T>
    void set_(T* ptr) { tagged.set(ptr, get_type_for<T>()); }

    template<typename T>
    T* get() const {
        assert(get_type_for<T>() == get_type());
        return static_cast<T*>(tagged.get_ptr());
    }
};

template<> constexpr StatementType StatementNode::get_type_for<PrintStatementNode>() { return StatementType::PRINT; }
template<> constexpr StatementType StatementNode::get_type_for<ExpressionStatementNode>() { return StatementType::EXPRESSION; }
template<> constexpr StatementType StatementNode::get_type_for<VariableDeclarationNode>() { return StatementType::VARIABLE; }
template<> constexpr StatementType StatementNode::get_type_for<BlockStatementNode>() { return StatementType::BLOCK; }
template<> constexpr StatementType StatementNode::get_type_for<IfStatementNode>() { return StatementType::IF; }
template<> constexpr StatementType StatementNode::get_type_for<WhileStatementNode>() { return StatementType::WHILE; }
template<> constexpr StatementType StatementNode::get_type_for<BreakStatementNode>() { return StatementType::BREAK; }
template<> constexpr StatementType StatementNode::get_type_for<ReturnStatementNode>() { return StatementType::RETURN; }
template<> constexpr StatementType StatementNode::get_type_for<FunctionDeclarationNode>() { return StatementType::FUNCTION; }


struct Program {
    std::string source;
    std::vector<Token> tokens;
    std::vector<StatementNode*> statements;
    ASTAllocator allocator;

    Program() = default;
    Program(Program&&) = default;
};
