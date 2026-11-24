#include "dlang/lexer.hpp"
#include "dlang/parser.hpp"

#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

using K = dlang::AstKind;

#define CHECK(condition) do { \
    if (!(condition)) { \
        throw std::runtime_error(std::string(__FILE__) + ':' + \
            std::to_string(__LINE__) + ": " #condition); \
    } \
} while (false)

dlang::AstNode parse(const std::string& source) {
    dlang::Lexer lexer(source, dlang::NewlineMode::Emit);
    return dlang::Parser(lexer.tokenize()).parse();
}

void expect_syntax_error(const std::string& source, std::size_t line,
                         std::size_t column, const std::string& message_part) {
    try {
        (void)parse(source);
    } catch (const dlang::SyntaxError& error) {
        CHECK(error.line() == line);
        CHECK(error.column() == column);
        CHECK(std::string(error.what()).find(message_part) != std::string::npos);
        return;
    }
    throw std::runtime_error("expected SyntaxError");
}

void empty_program_and_separators() {
    for (const std::string source : {"", "\n\n", ";;;", "\n;\n"}) {
        const auto tree = parse(source);
        CHECK(tree.kind == K::Program);
        CHECK(tree.children.empty());
    }
}

void declarations_assignments_and_references() {
    const auto tree = parse(
        "var a, b := 2\n"
        "a.1.2[3](4).name := b\n");
    CHECK(tree.children.size() == 2);
    CHECK(tree.children[0].kind == K::VariableDeclaration);
    CHECK(tree.children[0].children.size() == 2);
    CHECK(tree.children[0].children[0].value == "a");
    CHECK(tree.children[0].children[0].children.empty());
    CHECK(tree.children[0].children[1].value == "b");
    CHECK(tree.children[1].kind == K::Assignment);
    const auto& target = tree.children[1].children[0];
    CHECK(target.kind == K::MemberExpression);
    CHECK(target.value == "name");
    CHECK(target.children[0].kind == K::CallExpression);
}

void expression_precedence() {
    const auto tree = parse("var x := 1 + 2 * 3 < 8 and not false\n");
    const auto& logical = tree.children[0].children[0].children[0];
    CHECK(logical.kind == K::BinaryExpression);
    CHECK(logical.value == "and");
    CHECK(logical.children[0].value == "<");
    CHECK(logical.children[0].children[0].value == "+");
    CHECK(logical.children[0].children[0].children[1].value == "*");
    CHECK(logical.children[1].kind == K::UnaryExpression);
    CHECK(logical.children[1].value == "not");
}

void literals_functions_and_type_checks() {
    const auto tree = parse(
        "var data := [1, 2.5, true, false, none, {name := 'D', 7}]\n"
        "var f := func(x, y) => x + y\n"
        "var empty := func() is\n"
        "  return\n"
        "end\n"
        "print data is [], f is func, empty is func\n"
        "print data is int, data is real, data is bool, data is string, "
        "data is none, data is {}, data is []\n");
    CHECK(tree.children.size() == 5);
    const auto& array = tree.children[0].children[0].children[0];
    CHECK(array.kind == K::ArrayLiteral);
    CHECK(array.children.size() == 6);
    CHECK(array.children.back().kind == K::TupleLiteral);
    CHECK(array.children.back().children[0].value == "name");
    CHECK(tree.children[1].children[0].children[0].kind == K::FunctionLiteral);
    CHECK(tree.children[2].children[0].children[0].children[1].kind == K::BlockBody);
    CHECK(tree.children[3].kind == K::PrintStatement);
    CHECK(tree.children[3].children.size() == 3);
    for (const auto& expression : tree.children[3].children) {
        CHECK(expression.kind == K::TypeCheckExpression);
    }
    CHECK(tree.children[4].children.size() == 7);
    CHECK(tree.children[4].children[0].children[1].value == "int");
    CHECK(tree.children[4].children[5].children[1].value == "tuple");
    CHECK(tree.children[4].children[6].children[1].value == "array");
}

void conditionals() {
    const auto tree = parse(
        "if a >= 1 then\n"
        "  print 'yes'\n"
        "else\n"
        "  if b => print 'nested'\n"
        "end\n");
    CHECK(tree.children.size() == 1);
    const auto& statement = tree.children[0];
    CHECK(statement.kind == K::IfStatement);
    CHECK(statement.children.size() == 3);
    CHECK(statement.children[1].value == "then");
    CHECK(statement.children[2].value == "else");
    CHECK(statement.children[2].children[0].kind == K::ShortIfStatement);
}

void loops() {
    const auto tree = parse(
        "loop\n exit\nend\n"
        "while ready loop\n print ready\nend\n"
        "for 1..3 loop\n print 1\nend\n"
        "for item in values loop\n print item\nend\n");
    CHECK(tree.children.size() == 4);
    CHECK(tree.children[0].kind == K::InfiniteLoop);
    CHECK(tree.children[1].kind == K::WhileLoop);
    CHECK(tree.children[2].kind == K::ForLoop);
    CHECK(tree.children[2].value.empty());
    CHECK(tree.children[2].children[0].kind == K::RangeExpression);
    CHECK(tree.children[3].kind == K::ForLoop);
    CHECK(tree.children[3].value == "item");
}

void syntax_errors() {
    expect_syntax_error("var a := 1 var b := 2", 1, 12, "expected newline");
    expect_syntax_error("x + 1", 1, 3, "expected ':='");
    expect_syntax_error("if true then\nprint 1", 2, 8, "expected 'end'");
    expect_syntax_error("if true then\nend", 2, 1, "expected a statement");
    expect_syntax_error("var t := {}", 1, 11, "at least one element");
    expect_syntax_error("print 1 < 2 < 3", 1, 13, "chained comparison");
    expect_syntax_error("print a.", 1, 9, "tuple element name");
    expect_syntax_error("while true print 1", 1, 12, "expected 'loop'");
}

void ast_output() {
    std::ostringstream output;
    output << parse("var result := 1 + 2 * 3\nprint result\n");
    CHECK(output.str() ==
        "Program\n"
        "  VariableDeclaration\n"
        "    Variable \"result\"\n"
        "      BinaryExpression \"+\"\n"
        "        IntegerLiteral \"1\"\n"
        "        BinaryExpression \"*\"\n"
        "          IntegerLiteral \"2\"\n"
        "          IntegerLiteral \"3\"\n"
        "  PrintStatement\n"
        "    Identifier \"result\"\n");
}

void parser_contract() {
    bool rejected = false;
    try {
        dlang::Parser parser({});
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    CHECK(rejected);

    dlang::Lexer lexer("print 1", dlang::NewlineMode::Emit);
    dlang::Parser parser(lexer.tokenize());
    (void)parser.parse();
    rejected = false;
    try {
        (void)parser.parse();
    } catch (const std::logic_error&) {
        rejected = true;
    }
    CHECK(rejected);
}

} // namespace

int main() {
    const std::vector<std::pair<const char*, std::function<void()>>> tests{
        {"empty program and separators", empty_program_and_separators},
        {"declarations, assignments, and references", declarations_assignments_and_references},
        {"expression precedence", expression_precedence},
        {"literals, functions, and type checks", literals_functions_and_type_checks},
        {"full and short conditionals", conditionals},
        {"all loop forms", loops},
        {"syntax diagnostics", syntax_errors},
        {"stable AST output", ast_output},
        {"parser API contract", parser_contract},
    };

    std::size_t failed = 0;
    for (const auto& test : tests) {
        try {
            test.second();
            std::cout << "PASS " << test.first << '\n';
        } catch (const std::exception& error) {
            ++failed;
            std::cerr << "FAIL " << test.first << ": " << error.what() << '\n';
        }
    }
    std::cout << tests.size() - failed << '/' << tests.size()
              << " parser unit tests passed\n";
    return failed == 0 ? 0 : 1;
}
