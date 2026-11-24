#pragma once

#include "dlang/ast.hpp"
#include "dlang/token.hpp"

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <vector>

namespace dlang {

class SyntaxError : public std::runtime_error {
public:
    SyntaxError(const Token& token, const std::string& message);

    std::size_t line() const noexcept { return line_; }
    std::size_t column() const noexcept { return column_; }

private:
    std::size_t line_;
    std::size_t column_;
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    // Parse one complete program. The parser can be used only once.
    AstNode parse();

private:
    const Token& current() const noexcept;
    const Token& previous() const noexcept;
    TokenType lookahead_type(std::size_t distance) const noexcept;
    bool check(TokenType type) const noexcept;
    bool check_any(std::initializer_list<TokenType> types) const noexcept;
    bool match(TokenType type);
    bool match_any(std::initializer_list<TokenType> types);
    const Token& advance() noexcept;
    const Token& expect(TokenType type, const std::string& message);
    [[noreturn]] void error_here(const std::string& message) const;

    bool is_separator() const noexcept;
    void skip_separators() noexcept;
    bool can_start_expression() const noexcept;

    AstNode parse_block(std::initializer_list<TokenType> terminators,
                        const std::string& context);
    AstNode parse_statement();
    AstNode parse_declaration();
    AstNode parse_variable();
    AstNode parse_assignment();
    AstNode parse_if();
    AstNode parse_while();
    AstNode parse_for();
    AstNode parse_infinite_loop();
    AstNode parse_return();
    AstNode parse_print();

    AstNode parse_expression();
    AstNode parse_logical();
    AstNode parse_relation();
    AstNode parse_additive();
    AstNode parse_multiplicative();
    AstNode parse_unary();
    AstNode parse_primary();
    AstNode parse_reference();
    AstNode parse_array_literal();
    AstNode parse_tuple_literal();
    AstNode parse_function_literal();
    AstNode parse_type_indicator();

    std::vector<Token> tokens_;
    std::size_t index_ = 0;
    bool parsed_ = false;
};

} // namespace dlang
