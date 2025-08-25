#include <iostream>
#include<vector>

#include "antlr4-runtime.h"
#include"z3++.h"
#include "src/leaf_sts.hpp"
#include "src/prio_sts.hpp"
#include "src/rr_checker.hpp"
#include "src/utils.hpp"
#include "src/gen/constr_extractor.hpp"
#include "src/gen/fperfLexer.h"
#include "src/gen/fperfParser.h"
#include "src/gen/wl_parser.hpp"

class fperfVisitor;
using namespace std;
using namespace z3;
using namespace antlr4;

constexpr int MAX_ENQ = 4;
constexpr int MAX_DEQ = 1;
constexpr int TIME_STEPS = 10;
constexpr int NUM_PORTS = 3;
constexpr int PKT_TYPES = 1;
constexpr int BUFF_CAP = 10;

bool contains(vector<vector<int> > &container, vector<int> value) {
    if (ranges::find(container, value) != container.end()) {
        return true;
    } else {
        return false;
    }
}

expr add_constr(LeafSts *sts, map<tuple<int, int, int>, int> inp) {
    expr e = sts->slv.ctx.bool_val(true);
    for (const auto &[key, buf]: sts->buffs) {
        int src = get<0>(key);
        int dst = get<1>(key);
        for (int t = 0; t < TIME_STEPS; ++t) {
            int val = 0;
            if (inp.contains({src, dst, t})) {
                val = inp[{src, dst, t}];
                cout << "INCLUDES: " << src << " -> " << dst << " @" << t << endl;
            }
            e = e && (buf->I[t] == val);
        }
    }
    // for (int s = 0; s < sts->I.size(); ++s) {
    // for (int t = 0; t < sts->I[0].size(); ++t) {
    // for (int d = 0; d < sts->I[0][0].size(); ++d) {
    // int val = 0;
    // int src = s / sts->num_ports;
    // int dst = s % sts->num_ports;
    // if (inp.contains({src, dst, t})) {
    // val = inp[{src, dst, t}];
    // e = e && (I[s][t][d] == val);
    // cout << "INCLUDES: " << s << " -> " << d << " @" << t << endl;
    // }
    // e = e && (I[s][t][d] == val);
    // }
    // }
    // }
    return e;
}

int main(const int argc, const char *argv[]) {
    SmtSolver slv;
    LeafSts *l1;
    vector<tuple<int, int> > port_list = {
        {0, 1},
        {0, 2},
        {1, 0},
        {1, 2},
        {2, 0},
        {2, 1}
    };
    l1 = new LeafSts(slv, "leaf", port_list, TIME_STEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ);

    // LeafSts *l2;
    // l2 = new LeafSts(slv, "leaf", NUM_PORTS, TIME_STEPS, 5, MAX_ENQ, MAX_DEQ);

    map<tuple<int, int, int>, int> ins = {
        {{1, 0, 0}, 2},
        {{2, 0, 0}, 2},
        // {{2, 0, 0}, 0},
        // {{0, 1, 0}, 2},
        // {{2, 1, 0}, 2},
    };
    auto constr = add_constr(l1, ins);
    slv.add({constr, "inp"});

    auto base = l1->base_constrs();
    slv.add(base);

    auto mod = slv.check_sat();

    l1->print(mod);
}
