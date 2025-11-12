//
// Created by Amir Hossein Seyhani on 10/8/25.
//

#include "leaf_workload_parser.hpp"

#include <ANTLRInputStream.h>

#include "constr_extractor.hpp"
#include "fperfLexer.h"
#include "fperfParser.h"
#include <set>
#include "../leaf_utils.hpp"

using namespace antlr4;

#define DEBUG false

LeafWorkloadParser::LeafWorkloadParser(SmtSolver &slv, ev3 &I, int timesteps, map<int, int> pkt_type_to_dst,
                                       map<int, int> pkt_type_to_ecmp) : slv(slv),
                                                                         timesteps(timesteps),
                                                                         I(I) {
    for (auto &[pkt_type,dst]: pkt_type_to_dst) {
        dst_to_pkt_type[dst].push_back(pkt_type);
    }

    for (auto &[pkt_type,ecmp]: pkt_type_to_ecmp) {
        ecmp_to_pkt_type[ecmp].push_back(pkt_type);
    }
    for (int i = 0; i < I.size(); ++i) {
        max_t_with_zero_cenq.push_back(-1);
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

void LeafWorkloadParser::merge(vector<int> max_t_update) {
    for (int i = 0; i < max_t_update.size(); ++i) {
        max_t_with_zero_cenq[i] = max(max_t_with_zero_cenq[i], max_t_update[i]);
    }
}

set<int> LeafWorkloadParser::get_zero_inputs() {
    set<int> result = {};
    for (int i = 0; i < max_t_with_zero_cenq.size(); ++i) {
        if (max_t_with_zero_cenq[i] >= timesteps - 1)
            result.insert(i);
    }
    return result;
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
    merge(visitor->max_t_with_zero_cenq);
    dst_constrs.insert(dst_constrs.end(), visitor->dst_constrs.begin(), visitor->dst_constrs.end());
    ecmp_constrs.insert(ecmp_constrs.end(), visitor->ecmp_constrs.begin(), visitor->ecmp_constrs.end());
    vector<NamedExp> nes;
    for (auto &constr: visitor->constrs)
        nes.push_back(constr.prefix(prefix));

    // CENQ
    for (int i = 0; i < I.size(); ++i) {
        for (int j = 0; j < timesteps; ++j) {
            wl_expr = wl_expr && (sum(I[i][j]) == visitor->IT[i][j]);
        }
    }
    nes.emplace_back(wl_expr, format("{}: IT = sum(I)", prefix));

    return nes;
}

void LeafWorkloadParser::parse(vector<string> wl) {
    auto wl_expr = slv.ctx.bool_val(true);
    for (int i = 0; i < wl.size(); ++i) {
        string line = wl[i];
        string prefix = format("WL_{}", i);
        auto nes = parse(prefix, line);
        slv.add(nes);
    }

    for (int i = 0; i < max_t_with_zero_cenq.size(); ++i) {
        cout << i << ": " << max_t_with_zero_cenq[i] << endl;
    }

    int num_buffs = I.size();
    slv.add(uniq(I, slv, dst_to_pkt_type, num_buffs, timesteps));
    slv.add(valid(I, slv, num_buffs, timesteps));
    slv.add(same(I, slv, dst_to_pkt_type, num_buffs, timesteps));
}

expr LeafWorkloadParser::base_wl() {
}

expr LeafWorkloadParser::query() {
}
