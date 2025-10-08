#include "antlr4-runtime.h"
#include "../src/gen/fperfLexer.h"
#include "../src/gen/fperfParser.h"
#include "../src/gen/constr_extractor.hpp"
#include <iostream>

#include "../src/utils.hpp"

using namespace std;
using namespace antlr4;

int main(int argc, const char *argv[]) {
    SmtSolver slv;
    fperfVisitor *visitor = new ConstrExtractor(slv, 12, 10);
    vector<vector<string> > wls = read_wl_file("bar");
    for (auto wl: wls) {
        for (auto line: wl) {
            if (line.find_first_not_of(" \t\r\n") == std::string::npos)
                continue;
            try {
                ANTLRInputStream inputStream(line);

                fperfLexer lexer(&inputStream);
                CommonTokenStream tokens(&lexer);

                fperfParser parser(&tokens);
                auto tree = parser.con();

                visitor->visit(tree);
            } catch (const std::exception &e) {
                cout << "Error: " << e.what() << endl;
                cout << "Line: " << line << endl;
                throw;
            }
        }
    }
    return 1;
    // // string input = "[1, 10]: cenq(8, t) >= t";
    // string input = "[1, 10]: SUM_[q in {1,2}]cenq(q, t) >= t";
    // ANTLRInputStream inputStream(input);
    //
    // fperfLexer lexer(&inputStream);
    // CommonTokenStream tokens(&lexer);
    //
    // fperfParser parser(&tokens);
    // auto tree = parser.con();
    //
    // fperfVisitor *visitor = new ConstrExtractor();
    // visitor->visit(tree);
    // std::cout << "AST traversal completed successfully!" << std::endl;

    return 0;
}
