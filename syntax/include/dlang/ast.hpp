#pragma once

#include <cstddef>
#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

namespace dlang {

enum class AstKind {
    Program,
    Block,
    VariableDeclaration,
    Variable,
    Assignment,
    IfStatement,
    ShortIfStatement,
    WhileLoop,
    ForLoop,
    InfiniteLoop,
    ExitStatement,
    ReturnStatement,
    PrintStatement,
    ParameterList,
    Parameter,
    ExpressionBody,
    BlockBody,
    Identifier,
    IntegerLiteral,
    RealLiteral,
    StringLiteral,
    BooleanLiteral,
    NoneLiteral,
    ArrayLiteral,
    TupleLiteral,
    TupleElement,
    FunctionLiteral,
    UnaryExpression,
    BinaryExpression,
    TypeCheckExpression,
    TypeIndicator,
    IndexExpression,
    CallExpression,
    MemberExpression,
    TupleIndexExpression,
    RangeExpression,
};

struct AstNode {
    AstKind kind;
    std::string value;
    std::size_t line;
    std::size_t column;
    std::vector<AstNode> children;
};

std::string_view ast_kind_name(AstKind kind) noexcept;
void print_ast(std::ostream& out, const AstNode& node, std::size_t indent = 0);
std::ostream& operator<<(std::ostream& out, const AstNode& node);

} // namespace dlang
