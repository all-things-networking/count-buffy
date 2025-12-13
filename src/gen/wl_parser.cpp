//
// Created by Amir Hossein Seyhani on 7/9/25.
//

#include "wl_parser.hpp"
#include <format>
#include "../utils.hpp"

#include <ANTLRInputStream.h>
#include <vector>

#include "constr_extractor.hpp"
#include "fperfLexer.h"
#include "fperfParser.h"

using namespace antlr4;


WorkloadParser::WorkloadParser(ev3 &I, SmtSolver &slv, int num_buffs, int timesteps) : I(I), slv(slv),
    num_buffs(num_buffs), timesteps(timesteps) {
}


vector<NamedExp> WorkloadParser::parse(string prefix, string wl_line) {
    ANTLRInputStream inputStream(wl_line);
    fperfLexer lexer(&inputStream);
    CommonTokenStream tokens(&lexer);
    fperfParser parser(&tokens);
    auto tree = parser.con();
    ConstrExtractor *visitor = new ConstrExtractor(slv, I, num_buffs, timesteps);
    visitor->visit(tree);
    vector<NamedExp> nes;
    for (auto &constr: visitor->constrs)
        nes.push_back(constr.prefix(prefix));
    expr wl_expr = slv.ctx.bool_val(true);
    for (int i = 0; i < num_buffs; ++i)
        for (int j = 0; j < timesteps; ++j)
            wl_expr = wl_expr && (sum(I[i][j]) == visitor->IT[i][j]);
    nes.emplace_back(wl_expr, format("{}_SUM(I) = IT", prefix));
    return nes;
}

void WorkloadParser::parse(vector<string> wl) {
    for (int i = 0; i < wl.size(); ++i) {
        string line = wl[i];
        string prefix = format("WL_{}", i);
        auto nes = parse(prefix, line);
        slv.add(nes);
    }
}
