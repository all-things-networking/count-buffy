//
// Created by Amir Hossein Seyhani on 10/8/25.
//

#include "leaf_workload_parser.hpp"

#include <ANTLRInputStream.h>

#include "constr_extractor.hpp"
#include "fperfLexer.h"
#include "fperfParser.h"
#include "../leaf_utils.hpp"

using namespace antlr4;

#define DEBUG false

LeafWorkloadParser::LeafWorkloadParser(SmtSolver &slv, ev3 &I, int timesteps, map<int, int> pkt_type_to_dst,
                                       map<int, int> pkt_type_to_ecmp): slv(slv),
                                                                        timesteps(timesteps),
                                                                        I(I) {
    for (auto &[pkt_type,dst]: pkt_type_to_dst) {
        dst_to_pkt_type[dst].push_back(pkt_type);
    }

    for (auto &[pkt_type,ecmp]: pkt_type_to_ecmp) {
        ecmp_to_pkt_type[ecmp].push_back(pkt_type);
    }
    // for (int i = 0; i < num_spines; ++i) {
    //     for (int j = 0; j < num_leafs; ++j) {
    //         for (int k = 0; k < host_per_leaf; ++k) {
    //             int buf_idx = j * host_per_leaf + k;
    //             int pkt_type = i * host_per_spine + buf_idx;
    //             // cout << i << " " << j << " " << k << " " << pkt_type << endl;
    //             ecmp_to_pkt_type[i].push_back(pkt_type);
    //             dst_to_pkt_type[buf_idx].push_back(pkt_type);
    //             all_pkt_types.push_back(pkt_type);
    //         }
    //     }
    // }
}

vector<NamedExp> LeafWorkloadParser::parse(string prefix, string wl_line) {
    expr wl_expr = slv.ctx.bool_val(true);
    ANTLRInputStream inputStream(wl_line);
    fperfLexer lexer(&inputStream);
    CommonTokenStream tokens(&lexer);
    fperfParser parser(&tokens);
    auto tree = parser.con();
    ConstrExtractor *visitor = new ConstrExtractor(slv, I, timesteps, dst_to_pkt_type, ecmp_to_pkt_type);
    visitor->visit(tree);
    vector<NamedExp> nes;
    for (auto &constr: visitor->constrs)
        nes.push_back(constr.prefix(prefix));

    // CENQ
    // for (int i = 0; i < I.size(); ++i) {
    // for (int j = 0; j < timesteps; ++j) {
    // wl_expr = wl_expr && (sum(I[i][j]) == visitor->IT[i][j]);
    // }
    // }
    // nes.emplace_back(wl_expr, format("{}: IT = sum(I)", prefix));

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
    int num_buffs = I.size();
    slv.add(uniq(I, slv, dst_to_pkt_type, num_buffs, timesteps));
    slv.add(same(I, slv, dst_to_pkt_type, num_buffs, timesteps));
}

expr LeafWorkloadParser::base_wl() {
}

expr LeafWorkloadParser::query() {
}
