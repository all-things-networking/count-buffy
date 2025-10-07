//
// Created by Amir Hossein Seyhani on 7/9/25.
//

#include "wl_parser.hpp"
#include "../utils.hpp"

#include <ANTLRInputStream.h>
#include <vector>

#include "constr_extractor.hpp"
#include "fperfLexer.h"
#include "fperfParser.h"

using namespace antlr4;


WorkloadParser::WorkloadParser(ev3 &I, SmtSolver &slv, int n, int m) : I(I), slv(slv), n(n), m(m) {
}


expr WorkloadParser::parse(string prefix, string wl_line) {
    ANTLRInputStream inputStream(wl_line);
    fperfLexer lexer(&inputStream);
    CommonTokenStream tokens(&lexer);
    fperfParser parser(&tokens);
    auto tree = parser.con();
    ConstrExtractor *visitor = new ConstrExtractor(slv, n, m);
    visitor->visit(tree);
    expr wl_expr = slv.ctx.bool_val(true);
    for (int i = 0; i < visitor->constrs.size(); ++i)
        wl_expr = wl_expr && visitor->constrs[i];
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            wl_expr = wl_expr && (sum(I[i][j]) == visitor->IT[i][j]);
    return wl_expr;
}

void WorkloadParser::parse(vector<string> wl) {
    auto wl_expr = slv.ctx.bool_val(true);
    for (int i = 0; i < wl.size(); ++i) {
        string line = wl[i];
        string prefix = format("WL_{}", i);
        auto expr = parse(prefix, line);
        wl_expr = wl_expr && expr;
    }
    slv.add({wl_expr, "wl_expr"});
}
