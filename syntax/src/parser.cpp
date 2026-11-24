#include "dlang/parser.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace dlang {
namespace {

AstNode make_node(AstKind kind, const Token& token, std::string value = {}) {
    return {kind, std::move(value), token.line, token.column, {}};
}

AstNode make_binary(AstKind kind, const Token& operation,
                    AstNode left, AstNode right) {
    AstNode result = make_node(kind, operation, operation.lexeme);
    result.children.push_back(std::move(left));
    result.children.push_back(std::move(right));
    return result;
}

bool is_comparison(TokenType type) noexcept {
    switch (type) {
    case TokenType::Less:
    case TokenType::LessEqual:
    case TokenType::Greater:
    case TokenType::GreaterEqual:
    case TokenType::Equal:
    case TokenType::NotEqual:
        return true;
    default:
        return false;
    }
}

} // namespace

SyntaxError::SyntaxError(const Token& token, const std::string& message)
    : std::runtime_error("syntax error at " + std::to_string(token.line) + ':' +
                         std::to_string(token.column) + ": " + message +
                         "; got " + std::string(token_type_name(token.type))),
      line_(token.line), column_(token.column) {}

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {
    if (tokens_.empty() || tokens_.back().type != TokenType::EndOfFile) {
        throw std::invalid_argument("parser token stream must end with EOF");
    }
}

const Token& Parser::current() const noexcept {
    return tokens_[std::min(index_, tokens_.size() - 1)];
}

const Token& Parser::previous() const noexcept {
    return tokens_[index_ - 1];
}

TokenType Parser::lookahead_type(std::size_t distance) const noexcept {
    const std::size_t position = std::min(index_ + distance, tokens_.size() - 1);
    return tokens_[position].type;
}

bool Parser::check(TokenType type) const noexcept {
    return current().type == type;
}

bool Parser::check_any(std::initializer_list<TokenType> types) const noexcept {
    for (const TokenType type : types) {
        if (check(type)) {
            return true;
        }
    }
    return false;
}

bool Parser::match(TokenType type) {
    if (!check(type)) {
        return false;
    }
    advance();
    return true;
}

bool Parser::match_any(std::initializer_list<TokenType> types) {
    if (!check_any(types)) {
        return false;
    }
    advance();
    return true;
}

const Token& Parser::advance() noexcept {
    const Token& token = current();
    if (!check(TokenType::EndOfFile)) {
        ++index_;
    }
    return token;
}

const Token& Parser::expect(TokenType type, const std::string& message) {
    if (!check(type)) {
        error_here(message);
    }
    return advance();
}

[[noreturn]] void Parser::error_here(const std::string& message) const {
    throw SyntaxError(current(), message);
}

bool Parser::is_separator() const noexcept {
    return check(TokenType::Newline) || check(TokenType::Semicolon);
}

void Parser::skip_separators() noexcept {
    while (is_separator()) {
        advance();
    }
}

bool Parser::can_start_expression() const noexcept {
    switch (current().type) {
    case TokenType::Identifier:
    case TokenType::IntegerLiteral:
    case TokenType::RealLiteral:
    case TokenType::StringLiteral:
    case TokenType::True:
    case TokenType::False:
    case TokenType::None:
    case TokenType::Func:
    case TokenType::Plus:
    case TokenType::Minus:
    case TokenType::Not:
    case TokenType::LeftParen:
    case TokenType::LeftBracket:
    case TokenType::LeftBrace:
        return true;
    default:
        return false;
    }
}

AstNode Parser::parse() {
    if (parsed_) {
        throw std::logic_error("a Parser instance can parse only once");
    }
    parsed_ = true;

    AstNode program = make_node(AstKind::Program, current());
    skip_separators();
    while (!check(TokenType::EndOfFile)) {
        program.children.push_back(parse_statement());
        if (check(TokenType::EndOfFile)) {
            break;
        }
        if (!is_separator()) {
            error_here("expected newline or ';' between statements");
        }
        skip_separators();
    }
    return program;
}

AstNode Parser::parse_block(std::initializer_list<TokenType> terminators,
                            const std::string& context) {
    AstNode block = make_node(AstKind::Block, current());
    skip_separators();
    if (check_any(terminators) || check(TokenType::EndOfFile)) {
        error_here("expected a statement in " + context);
    }

    while (!check_any(terminators) && !check(TokenType::EndOfFile)) {
        block.children.push_back(parse_statement());
        if (check_any(terminators) || check(TokenType::EndOfFile)) {
            break;
        }
        if (!is_separator()) {
            error_here("expected newline or ';' between statements");
        }
        skip_separators();
    }
    return block;
}

AstNode Parser::parse_statement() {
    switch (current().type) {
    case TokenType::Var: return parse_declaration();
    case TokenType::Identifier: return parse_assignment();
    case TokenType::If: return parse_if();
    case TokenType::While: return parse_while();
    case TokenType::For: return parse_for();
    case TokenType::Loop: return parse_infinite_loop();
    case TokenType::Exit: {
        const Token token = advance();
        return make_node(AstKind::ExitStatement, token);
    }
    case TokenType::Return: return parse_return();
    case TokenType::Print: return parse_print();
    default:
        error_here("expected a statement");
    }
}

AstNode Parser::parse_declaration() {
    const Token keyword = expect(TokenType::Var, "expected 'var'");
    AstNode declaration = make_node(AstKind::VariableDeclaration, keyword);
    declaration.children.push_back(parse_variable());
    while (match(TokenType::Comma)) {
        declaration.children.push_back(parse_variable());
    }
    return declaration;
}

AstNode Parser::parse_variable() {
    const Token name = expect(TokenType::Identifier,
                              "expected variable name after 'var' or ','");
    AstNode variable = make_node(AstKind::Variable, name, name.lexeme);
    if (match(TokenType::Assign)) {
        variable.children.push_back(parse_expression());
    }
    return variable;
}

AstNode Parser::parse_assignment() {
    const Token start = current();
    AstNode assignment = make_node(AstKind::Assignment, start);
    assignment.children.push_back(parse_reference());
    expect(TokenType::Assign, "expected ':=' after assignment target");
    assignment.children.push_back(parse_expression());
    return assignment;
}

AstNode Parser::parse_if() {
    const Token keyword = expect(TokenType::If, "expected 'if'");
    AstNode condition = parse_expression();

    if (match(TokenType::Then)) {
        AstNode statement = make_node(AstKind::IfStatement, keyword);
        statement.children.push_back(std::move(condition));
        AstNode then_block = parse_block({TokenType::Else, TokenType::End},
                                         "'then' branch");
        then_block.value = "then";
        statement.children.push_back(std::move(then_block));
        if (match(TokenType::Else)) {
            AstNode else_block = parse_block({TokenType::End}, "'else' branch");
            else_block.value = "else";
            statement.children.push_back(std::move(else_block));
        }
        expect(TokenType::End, "expected 'end' after conditional statement");
        return statement;
    }

    if (match(TokenType::Arrow)) {
        AstNode statement = make_node(AstKind::ShortIfStatement, keyword);
        statement.children.push_back(std::move(condition));
        AstNode body = make_node(AstKind::Block, current(), "then");
        body.children.push_back(parse_statement());
        statement.children.push_back(std::move(body));
        return statement;
    }

    error_here("expected 'then' or '=>' after if condition");
}

AstNode Parser::parse_while() {
    const Token keyword = expect(TokenType::While, "expected 'while'");
    AstNode statement = make_node(AstKind::WhileLoop, keyword);
    statement.children.push_back(parse_expression());
    expect(TokenType::Loop, "expected 'loop' after while condition");
    AstNode body = parse_block({TokenType::End}, "while loop body");
    body.value = "body";
    statement.children.push_back(std::move(body));
    expect(TokenType::End, "expected 'end' after while loop body");
    return statement;
}

AstNode Parser::parse_for() {
    const Token keyword = expect(TokenType::For, "expected 'for'");
    AstNode statement = make_node(AstKind::ForLoop, keyword);

    if (check(TokenType::Identifier) && lookahead_type(1) == TokenType::In) {
        statement.value = advance().lexeme;
        advance(); // in
    }

    AstNode sequence = parse_expression();
    if (match(TokenType::Range)) {
        const Token operation = previous();
        AstNode range = make_node(AstKind::RangeExpression, operation, operation.lexeme);
        range.children.push_back(std::move(sequence));
        range.children.push_back(parse_expression());
        sequence = std::move(range);
    }
    statement.children.push_back(std::move(sequence));

    expect(TokenType::Loop, "expected 'loop' after for-loop header");
    AstNode body = parse_block({TokenType::End}, "for loop body");
    body.value = "body";
    statement.children.push_back(std::move(body));
    expect(TokenType::End, "expected 'end' after for loop body");
    return statement;
}

AstNode Parser::parse_infinite_loop() {
    const Token keyword = expect(TokenType::Loop, "expected 'loop'");
    AstNode statement = make_node(AstKind::InfiniteLoop, keyword);
    AstNode body = parse_block({TokenType::End}, "loop body");
    body.value = "body";
    statement.children.push_back(std::move(body));
    expect(TokenType::End, "expected 'end' after loop body");
    return statement;
}

AstNode Parser::parse_return() {
    const Token keyword = expect(TokenType::Return, "expected 'return'");
    AstNode statement = make_node(AstKind::ReturnStatement, keyword);
    if (can_start_expression()) {
        statement.children.push_back(parse_expression());
    }
    return statement;
}

AstNode Parser::parse_print() {
    const Token keyword = expect(TokenType::Print, "expected 'print'");
    AstNode statement = make_node(AstKind::PrintStatement, keyword);
    statement.children.push_back(parse_expression());
    while (match(TokenType::Comma)) {
        statement.children.push_back(parse_expression());
    }
    return statement;
}

AstNode Parser::parse_expression() {
    return parse_logical();
}

AstNode Parser::parse_logical() {
    AstNode left = parse_relation();
    while (match_any({TokenType::Or, TokenType::And, TokenType::Xor})) {
        const Token operation = previous();
        left = make_binary(AstKind::BinaryExpression, operation,
                           std::move(left), parse_relation());
    }
    return left;
}

AstNode Parser::parse_relation() {
    AstNode left = parse_additive();
    if (is_comparison(current().type)) {
        const Token operation = advance();
        left = make_binary(AstKind::BinaryExpression, operation,
                           std::move(left), parse_additive());
        if (is_comparison(current().type)) {
            error_here("chained comparison operators are not allowed");
        }
    }
    return left;
}

AstNode Parser::parse_additive() {
    AstNode left = parse_multiplicative();
    while (match_any({TokenType::Plus, TokenType::Minus})) {
        const Token operation = previous();
        left = make_binary(AstKind::BinaryExpression, operation,
                           std::move(left), parse_multiplicative());
    }
    return left;
}

AstNode Parser::parse_multiplicative() {
    AstNode left = parse_unary();
    while (match_any({TokenType::Star, TokenType::Slash})) {
        const Token operation = previous();
        left = make_binary(AstKind::BinaryExpression, operation,
                           std::move(left), parse_unary());
    }
    return left;
}

AstNode Parser::parse_unary() {
    if (match_any({TokenType::Plus, TokenType::Minus, TokenType::Not})) {
        const Token operation = previous();
        AstNode expression = make_node(AstKind::UnaryExpression, operation,
                                       operation.lexeme);
        expression.children.push_back(parse_unary());
        return expression;
    }

    if (check(TokenType::Identifier)) {
        AstNode reference = parse_reference();
        if (match(TokenType::Is)) {
            const Token operation = previous();
            AstNode expression = make_node(AstKind::TypeCheckExpression, operation,
                                           operation.lexeme);
            expression.children.push_back(std::move(reference));
            expression.children.push_back(parse_type_indicator());
            return expression;
        }
        return reference;
    }

    return parse_primary();
}

AstNode Parser::parse_primary() {
    if (match(TokenType::IntegerLiteral)) {
        return make_node(AstKind::IntegerLiteral, previous(), previous().lexeme);
    }
    if (match(TokenType::RealLiteral)) {
        return make_node(AstKind::RealLiteral, previous(), previous().lexeme);
    }
    if (match(TokenType::StringLiteral)) {
        return make_node(AstKind::StringLiteral, previous(), previous().lexeme);
    }
    if (match_any({TokenType::True, TokenType::False})) {
        return make_node(AstKind::BooleanLiteral, previous(), previous().lexeme);
    }
    if (match(TokenType::None)) {
        return make_node(AstKind::NoneLiteral, previous(), previous().lexeme);
    }
    if (check(TokenType::LeftBracket)) {
        return parse_array_literal();
    }
    if (check(TokenType::LeftBrace)) {
        return parse_tuple_literal();
    }
    if (check(TokenType::Func)) {
        return parse_function_literal();
    }
    if (match(TokenType::LeftParen)) {
        AstNode expression = parse_expression();
        expect(TokenType::RightParen, "expected ')' after parenthesized expression");
        return expression;
    }
    error_here("expected an expression");
}

AstNode Parser::parse_reference() {
    const Token name = expect(TokenType::Identifier, "expected identifier");
    AstNode reference = make_node(AstKind::Identifier, name, name.lexeme);

    while (true) {
        if (match(TokenType::LeftBracket)) {
            const Token opening = previous();
            AstNode expression = make_node(AstKind::IndexExpression, opening);
            expression.children.push_back(std::move(reference));
            expression.children.push_back(parse_expression());
            expect(TokenType::RightBracket, "expected ']' after array index");
            reference = std::move(expression);
            continue;
        }

        if (match(TokenType::LeftParen)) {
            const Token opening = previous();
            AstNode expression = make_node(AstKind::CallExpression, opening);
            expression.children.push_back(std::move(reference));
            if (!check(TokenType::RightParen)) {
                expression.children.push_back(parse_expression());
                while (match(TokenType::Comma)) {
                    expression.children.push_back(parse_expression());
                }
            }
            expect(TokenType::RightParen, "expected ')' after function arguments");
            reference = std::move(expression);
            continue;
        }

        if (match(TokenType::Dot)) {
            const Token dot = previous();
            if (match(TokenType::Identifier)) {
                AstNode expression = make_node(AstKind::MemberExpression, dot,
                                               previous().lexeme);
                expression.children.push_back(std::move(reference));
                reference = std::move(expression);
                continue;
            }
            if (match(TokenType::IntegerLiteral)) {
                AstNode expression = make_node(AstKind::TupleIndexExpression, dot,
                                               previous().lexeme);
                expression.children.push_back(std::move(reference));
                reference = std::move(expression);
                continue;
            }
            error_here("expected tuple element name or integer position after '.'");
        }
        break;
    }
    return reference;
}

AstNode Parser::parse_array_literal() {
    const Token opening = expect(TokenType::LeftBracket, "expected '['");
    AstNode array = make_node(AstKind::ArrayLiteral, opening);
    if (!check(TokenType::RightBracket)) {
        array.children.push_back(parse_expression());
        while (match(TokenType::Comma)) {
            array.children.push_back(parse_expression());
        }
    }
    expect(TokenType::RightBracket, "expected ']' after array literal");
    return array;
}

AstNode Parser::parse_tuple_literal() {
    const Token opening = expect(TokenType::LeftBrace, "expected '{'");
    AstNode tuple = make_node(AstKind::TupleLiteral, opening);
    if (check(TokenType::RightBrace)) {
        error_here("tuple literal must contain at least one element");
    }

    do {
        const Token start = current();
        AstNode element = make_node(AstKind::TupleElement, start);
        if (check(TokenType::Identifier) && lookahead_type(1) == TokenType::Assign) {
            element.value = advance().lexeme;
            advance(); // :=
        }
        element.children.push_back(parse_expression());
        tuple.children.push_back(std::move(element));
    } while (match(TokenType::Comma));

    expect(TokenType::RightBrace, "expected '}' after tuple literal");
    return tuple;
}

AstNode Parser::parse_function_literal() {
    const Token keyword = expect(TokenType::Func, "expected 'func'");
    AstNode function = make_node(AstKind::FunctionLiteral, keyword);
    AstNode parameters = make_node(AstKind::ParameterList, keyword);

    if (match(TokenType::LeftParen)) {
        if (!check(TokenType::RightParen)) {
            do {
                const Token parameter = expect(TokenType::Identifier,
                                               "expected function parameter name");
                parameters.children.push_back(
                    make_node(AstKind::Parameter, parameter, parameter.lexeme));
            } while (match(TokenType::Comma));
        }
        expect(TokenType::RightParen, "expected ')' after function parameters");
    }
    function.children.push_back(std::move(parameters));

    if (match(TokenType::Arrow)) {
        const Token arrow = previous();
        AstNode body = make_node(AstKind::ExpressionBody, arrow);
        body.children.push_back(parse_expression());
        function.children.push_back(std::move(body));
        return function;
    }

    if (match(TokenType::Is)) {
        const Token is = previous();
        AstNode body = make_node(AstKind::BlockBody, is);
        body.children.push_back(parse_block({TokenType::End}, "function body"));
        expect(TokenType::End, "expected 'end' after function body");
        function.children.push_back(std::move(body));
        return function;
    }

    error_here("expected '=>' or 'is' after function parameters");
}

AstNode Parser::parse_type_indicator() {
    const Token token = current();
    switch (token.type) {
    case TokenType::IntType:
        advance();
        return make_node(AstKind::TypeIndicator, token, "int");
    case TokenType::RealType:
        advance();
        return make_node(AstKind::TypeIndicator, token, "real");
    case TokenType::BoolType:
        advance();
        return make_node(AstKind::TypeIndicator, token, "bool");
    case TokenType::StringType:
        advance();
        return make_node(AstKind::TypeIndicator, token, "string");
    case TokenType::None:
        advance();
        return make_node(AstKind::TypeIndicator, token, "none");
    case TokenType::Func:
        advance();
        return make_node(AstKind::TypeIndicator, token, "func");
    case TokenType::LeftBracket:
        advance();
        expect(TokenType::RightBracket, "expected ']' in array type indicator");
        return make_node(AstKind::TypeIndicator, token, "array");
    case TokenType::LeftBrace:
        advance();
        expect(TokenType::RightBrace, "expected '}' in tuple type indicator");
        return make_node(AstKind::TypeIndicator, token, "tuple");
    default:
        error_here("expected type indicator after 'is'");
    }
}

} // namespace dlang
