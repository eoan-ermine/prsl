#include <iostream>

#include "prsl/Scanner/scanner.hpp"
#include "prsl/Parser/parser.hpp"
#include <fstream>
#include <iterator>

void run(std::string_view source) {
    Scanner scanner(source);
    auto tokens = scanner.tokenize();
    prsl::Errors::ErrorReporter eReporter;
    prsl::Parser::Parser parser(tokens, eReporter);
    auto statements = parser.parse();

    if (eReporter.getStatus() != prsl::Errors::PrslStatus::OK) {
        eReporter.printToErr();
    }
}

void runFile(std::string_view path) {
    std::ifstream file{path.data()};
    std::string source{std::istream_iterator<char>{file},
                            std::istream_iterator<char>{}};
    run(source);
}

void runPrompt() {
    for (std::string line; std::getline(std::cin, line);) {
        run(line);
    }
}

int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cout << "Usage: prsl [script]";
        return 64;
    } else if (argc == 2) {
        runFile(argv[0]);
    } else {
        runPrompt();
    }
}