#include "dlang/interpreter.hpp"
#include "execution.hpp"

#include <ostream>
#include <stdexcept>
#include <vector>

namespace dlang {
namespace {

std::vector<Value> iterationValues(const SemanticNode& sequence, const Value& aggregate) {
    std::vector<Value> values;
    if (const auto* array = std::get_if<std::shared_ptr<Array>>(&aggregate)) {
        for (const auto& element : (*array)->elements) {
            values.push_back(element.second);
        }
    } else if (const auto* tuple = std::get_if<std::shared_ptr<Tuple>>(&aggregate)) {
        for (const auto& field : (*tuple)->elements) {
            values.push_back(field.second);
        }
    } else {
        throw RuntimeError(sequence, "for sequence must be array or tuple");
    }
    return values;
}

} // namespace

void Interpreter::run(const SemanticModel& program) const {
    Execution(output_).run(program.root);
}

void Execution::run(const SemanticNode& program) {
    executeBlock(program, std::make_shared<Environment>());
}

StatementResult Execution::executeBlock(const SemanticNode& block, const Scope& scope) {
    for (const auto& statement : block.children) {
        auto result = executeStatement(statement, scope);
        if (result.flow != Flow::Next) {
            return result;
        }
    }
    return {};
}

StatementResult Execution::executeStatement(const SemanticNode& node, const Scope& scope) {
    switch (node.kind) {
    case AstKind::VariableDeclaration:
        for (const auto& variable : node.children) {
            // A recursive function captures the cell that its initializer will fill.
            Value& cell = scope->declare(variable.symbol_id);
            if (!variable.children.empty()) {
                cell = evaluateExpression(variable.children[0], scope);
            }
        }
        return {};
    case AstKind::Assignment:
        assign(node.children[0], node.children[1], scope);
        return {};
    case AstKind::PrintStatement:
        printValues(node, scope);
        return {};
    case AstKind::IfStatement:
    case AstKind::ShortIfStatement: {
        const auto& condition = node.children[0];
        bool takeThen = requireType<bool>(evaluateExpression(condition, scope),
                                          condition, "if condition must be bool");
        if (takeThen) {
            return executeBlock(node.children[1], std::make_shared<Environment>(scope));
        }
        if (node.children.size() == 3) {
            return executeBlock(node.children[2], std::make_shared<Environment>(scope));
        }
        return {};
    }
    case AstKind::WhileLoop:
    case AstKind::InfiniteLoop:
        return executeLoop(node, scope);
    case AstKind::ForLoop:
        return executeFor(node, scope);
    case AstKind::ExitStatement:
        return {Flow::Exit, {}};
    case AstKind::ReturnStatement:
        return {Flow::Return, node.children.empty() ? Value{} :
                              evaluateExpression(node.children[0], scope)};
    default:
        throw std::logic_error("expected a statement in the analyzed tree");
    }
}

StatementResult Execution::executeLoop(const SemanticNode& node, const Scope& scope) {
    const auto& body = node.children.back();
    while (true) {
        if (node.kind == AstKind::WhileLoop) {
            const auto& condition = node.children[0];
            if (!requireType<bool>(evaluateExpression(condition, scope),
                                   condition, "while condition must be bool")) {
                return {};
            }
        }
        auto result = executeBlock(body, std::make_shared<Environment>(scope));
        if (result.flow == Flow::Exit) {
            return {};
        }
        if (result.flow == Flow::Return) {
            return result;
        }
    }
}

StatementResult Execution::executeFor(const SemanticNode& node, const Scope& scope) {
    const auto& sequence = node.children[0];
    const auto& body = node.children[1];
    auto runIteration = [&](const Value& value) {
        auto locals = std::make_shared<Environment>(scope);
        if (node.symbol_id != 0) {
            locals->declare(node.symbol_id, value);
        }
        return executeBlock(body, locals);
    };

    StatementResult result;
    if (sequence.kind == AstKind::RangeExpression) {
        const auto& first = sequence.children[0];
        const auto& last = sequence.children[1];
        auto current = requireType<std::int64_t>(evaluateExpression(first, scope),
                                                 first, "range start must be int");
        const auto end = requireType<std::int64_t>(evaluateExpression(last, scope),
                                                   last, "range end must be int");
        while (current <= end) {
            result = runIteration(current);
            if (result.flow != Flow::Next || current == end) {
                break;
            }
            ++current;
        }
    } else {
        const auto snapshot = iterationValues(sequence, evaluateExpression(sequence, scope));
        for (const auto& value : snapshot) {
            result = runIteration(value);
            if (result.flow != Flow::Next) {
                break;
            }
        }
    }
    return result.flow == Flow::Return ? result : StatementResult{};
}

void Execution::printValues(const SemanticNode& node, const Scope& scope) {
    std::string line;
    for (std::size_t i = 0; i < node.children.size(); ++i) {
        if (i != 0) {
            line += ' ';
        }
        line += formatValue(evaluateExpression(node.children[i], scope));
    }
    output_ << line << '\n';
    if (!output_) {
        throw RuntimeError(node, "cannot write program output");
    }
}

} // namespace dlang
