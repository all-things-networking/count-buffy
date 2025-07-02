#include "antlr4-runtime.h"
#include "src/gen/fperfLexer.h"
#include "src/gen/fperfParser.h"
#include "src/gen/ASTTypeVisitor.cpp"
#include <iostream>

using namespace std;
using namespace antlr4;
using namespace antlrcpptest;

int main(int argc, const char *argv[]) {
    try {
        // Create an input stream from a string (you can also use file input)
        string input = "[1, 10]: cenq(8, t) >= t";
        ANTLRInputStream inputStream(input);

        // Create a lexer
        fperfLexer lexer(&inputStream);
        CommonTokenStream tokens(&lexer);

        // Create a parser
        fperfParser parser(&tokens);
        auto tree = parser.con();

        ASTTypeVisitor visitor;

        // Visit the parse tree
        visitor.visit(tree);

        std::cout << "AST traversal completed successfully!" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
