#pragma once

#include <iosfwd>

namespace dlang {

struct SemanticModel;

class Interpreter {
public:
    explicit Interpreter(std::ostream& output) : output_(output) {}
    void run(const SemanticModel& program) const;

private:
    std::ostream& output_;
};

} // namespace dlang
