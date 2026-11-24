#pragma once

#include "dlang/semantic.hpp"
#include "environment.hpp"
#include "runtime_objects.hpp"

#include <iosfwd>
#include <string>

namespace dlang {

using Scope = std::shared_ptr<Environment>;

enum class Flow { Next, Exit, Return };

struct StatementResult {
    Flow flow = Flow::Next;
    Value value;
};

template <typename T>
T requireType(const Value& value, const SemanticNode& node, const char* requirement) {
    if (const auto* result = std::get_if<T>(&value)) {
        return *result;
    }
    throw RuntimeError(node, std::string(requirement) + ", got " +
                             std::string(valueTypeName(value)));
}

class Execution {
public:
    explicit Execution(std::ostream& output) : output_(output) {}
    void run(const SemanticNode& program);

private:
    static constexpr unsigned maxCallDepth = 67;
    unsigned callDepth_ = 0;
    std::ostream& output_;
    RuntimeObjects objects_;

    Value evaluateExpression(const SemanticNode& node, const Scope& scope);
    Value callFunction(const SemanticNode& call, const Scope& caller);
    void assign(const SemanticNode& target, const SemanticNode& expression, const Scope& scope);

    struct ArrayLocation {
        std::shared_ptr<Array> array;
        std::int64_t key;
    };
    ArrayLocation arrayLocation(const SemanticNode& node, const Scope& scope);

    StatementResult executeStatement(const SemanticNode& node, const Scope& scope);
    StatementResult executeBlock(const SemanticNode& block, const Scope& scope);
    StatementResult executeLoop(const SemanticNode& node, const Scope& scope);
    StatementResult executeFor(const SemanticNode& node, const Scope& scope);
    void printValues(const SemanticNode& node, const Scope& scope);
};

} // namespace dlang
