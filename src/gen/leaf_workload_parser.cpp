//
// Created by Amir Hossein Seyhani on 10/8/25.
//

#include "leaf_workload_parser.hpp"
#include <format>

#include <ANTLRInputStream.h>

#include "constr_extractor.hpp"
#include "fperfLexer.h"
#include "fperfParser.h"
#include "../leaf_utils.hpp"

using namespace antlr4;

#define DEBUG false

LeafWorkloadParser::LeafWorkloadParser(SmtSolver &slv, ev3 &I, int num_spines, int num_leafs, int host_per_leaf,
                                       int timesteps): slv(slv),
                                                       timesteps(timesteps),
                                                       num_spines(num_spines),
                                                       num_leafs(num_leafs),
                                                       host_per_leaf(host_per_leaf),
                                                       I(I) {
    this->num_buffs = num_leafs * host_per_leaf;
    int host_per_spine = host_per_leaf * num_leafs;
    for (int i = 0; i < num_spines; ++i) {
        for (int j = 0; j < num_leafs; ++j) {
            for (int k = 0; k < host_per_leaf; ++k) {
                int buf_idx = j * host_per_leaf + k;
                int pkt_type = i * host_per_spine + buf_idx;
                // cout << i << " " << j << " " << k << " " << pkt_type << endl;
                ecmp_to_pkt_type[i].push_back(pkt_type);
                dst_to_pkt_type[buf_idx].push_back(pkt_type);
                all_pkt_types.push_back(pkt_type);
            }
        }
    }
}

vector<NamedExp> LeafWorkloadParser::parse(string prefix, string wl_line) {
    expr wl_expr = slv.ctx.bool_val(true);
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

    // CENQ
    for (int i = 0; i < num_buffs; ++i) {
        for (int j = 0; j < timesteps; ++j) {
            wl_expr = wl_expr && (sum(I[i][j]) == visitor->IT[i][j]);
        }
    }
    nes.emplace_back(wl_expr, format("{}: IT = sum(I)", prefix));

    // for (int i = 0; i < num_buffs; ++i) {
    // for (int t = 0; t < timesteps; ++t) {
    // int dst = visitor->DST[i][t];
    // dst_val(i, t) == dst
    // auto pkt_types = dst_to_pkt_type[dst];
    // for (auto pt: pkt_types) {
    //     if (ranges::find(pkt_types, pt) != pkt_types.end()) {
    //         auto expr = I[i][j][pt] == 0;
    //         constrs.emplace_back(expr, format("DST_{}_{}_{}_{}", i, j, dst, pt));
    //     }
    // }
    // }
    // }

    return nes;

    //
    // if (DEBUG) {
    //     slv.s.push();
    //     slv.add({wl_expr, "WL"});
    //     auto m = slv.check_sat();
    //     slv.s.pop();
    //     visitor->print(m);
    // }
    // return wl_expr;
    // return wl_expr;

    // for (int i = 0; i < num_buffs; ++i) {
    //     for (int j = 0; j < timesteps; ++j) {
    //         int ecmp = visitor->ECMP[i][j];
    //         auto pkt_types = ecmp_to_pkt_type[ecmp];
    //         for (auto pt: pkt_types) {
    //             if (ranges::find(pkt_types, pt) != pkt_types.end()) {
    //                 auto expr = I[i][j][pt] == 0;
    //                 constrs.emplace_back(expr, format("ECMP_{}_{}_{}_{}", i, j, ecmp, pt));
    //             }
    //         }
    //     }
    // }

    // wl_expr = wl_expr && (sum(I[i][j]) == visitor->IT[i][j]);
    // return wl_expr;
}

void LeafWorkloadParser::parse(vector<string> wl) {
    auto wl_expr = slv.ctx.bool_val(true);
    for (int i = 0; i < wl.size(); ++i) {
        string line = wl[i];
        string prefix = format("WL_{}", i);
        auto nes = parse(prefix, line);
        slv.add(nes);
    }
    slv.add(uniq(I, slv, num_buffs, timesteps));
    slv.add(same(I, slv, num_buffs, timesteps));
}

expr LeafWorkloadParser::base_wl() {
}

expr LeafWorkloadParser::query() {
}
