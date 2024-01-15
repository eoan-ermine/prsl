#include <iostream>

#include "scanner.hpp"
#include "parser.hpp"
#include <fstream>
#include <iterator>

void run(std::string_view source) {
    Scanner scanner(source);
    auto tokens = scanner.scanTokens();
    Parser parser(tokens);
    auto statements = parser.parse();
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