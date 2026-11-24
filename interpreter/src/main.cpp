#include "dlang/interpreter.hpp"
#include "dlang/lexer.hpp"
#include "dlang/optimizer.hpp"
#include "dlang/parser.hpp"
#include "dlang/semantic.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

std::string readSource(const char* path) {
    std::error_code error;
    if (std::filesystem::is_directory(path, error)) {
        throw std::runtime_error("cannot read source file: path is a directory");
    }
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("cannot open source file");
    }
    input.exceptions(std::ios::badbit);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

} // namespace

int main(int argc, char* argv[]) {
    const bool optimize = argc > 1 && std::string_view(argv[1]) == "--optimize";
    const int pathIndex = optimize ? 2 : 1;
    if (argc != pathIndex + 1 || argv[pathIndex][0] == '-' || argv[pathIndex][0] == '\0') {
        std::cerr << "Usage: " << argv[0] << " [--optimize] <source.d>\n";
        return 2;
    }
    const char* path = argv[pathIndex];

    try {
        dlang::Lexer lexer(readSource(path), dlang::NewlineMode::Emit);
        dlang::Parser parser(lexer.tokenize());
        dlang::AstNode ast = parser.parse();
        if (optimize) {
            ast = dlang::Optimizer().optimize(ast);
        }
        const dlang::SemanticModel model = dlang::SemanticAnalyzer().analyze(ast);
        dlang::Interpreter(std::cout).run(model);
        if (!std::cout.flush()) {
            throw std::runtime_error("cannot write program output");
        }
    } catch (const std::runtime_error& error) {
        std::cerr << path << ": " << error.what() << '\n';
        return 1;
    }
    return 0;
}
