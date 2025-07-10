//
// Created by Amir Hossein Seyhani on 7/9/25.
//

#include "wl_parser.hpp"

#include <ANTLRInputStream.h>
#include <vector>

#include "constr_extractor.hpp"
#include "fperfLexer.h"
#include "fperfParser.h"

using namespace antlr4;


WorkloadParser::WorkloadParser(ev3 &I, SmtSolver &slv, int n, int m) : I(I), slv(slv), n(n), m(m) {
}


void WorkloadParser::parse(string prefix, string wl_line) {
    ANTLRInputStream inputStream(wl_line);
    fperfLexer lexer(&inputStream);
    CommonTokenStream tokens(&lexer);
    fperfParser parser(&tokens);
    auto tree = parser.con();
    ConstrExtractor *visitor = new ConstrExtractor(slv, n, m);
    visitor->visit(tree);
    for (int i = 0; i < visitor->constrs.size(); ++i)
        slv.add(visitor->constrs[i], format("{}, constr {}", prefix, i));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            slv.add(sum(I[i][j]) == visitor->IT[i][j], format("{}, I[{}][{}] workload total", prefix, i, j));
}

void WorkloadParser::parse(vector<string> wl) {
    for (int i = 0; i < wl.size(); ++i) {
        string line = wl[i];
        string prefix = format("workload_line {}", i);
        parse(prefix, line);
    }
}
