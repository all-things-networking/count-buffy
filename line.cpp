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

expr link_ports(ev2 out, ev2 in) {
    auto e = out[0][0].ctx().bool_val(true);
    for (int i = 0; i < out.size(); ++i) {
        for (int j = 0; j < out[0].size(); ++j) {
            e = e && (out[i][j] == in[i][j]);
        }
    }
    return e;
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
    return e;
}

int main(const int argc, const char *argv[]) {
    SmtSolver slv;
    LeafSts *l1;
    vector<tuple<int, int> > l1_ports = {
        {0, 1},
    };
    l1 = new LeafSts(slv, "l1", l1_ports, TIME_STEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ);

    LeafSts *l2;
    vector<tuple<int, int> > l2_ports = {
        {0, 1},
    };
    l2 = new LeafSts(slv, "l2", l2_ports, TIME_STEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ);

    map<tuple<int, int, int>, int> ins = {
        {{0, 1, 0}, 2},
        // {{2, 0, 0}, 2},
        // {{2, 0, 0}, 0},
        // {{0, 1, 0}, 2},
        // {{2, 1, 0}, 2},
    };
    auto constr = add_constr(l1, ins);
    slv.add({constr, "inp"});

    auto base1 = l1->base_constrs();
    auto base1_merged = merge(base1, "base1");
    slv.add(base1_merged);

    auto base2 = l2->base_constrs();
    auto base2_merged = merge(base2, "base2");
    slv.add(base2_merged);

    slv.add(link_ports(l1->get_out_port(1), l2->get_in_port(0)), "link");

    auto mod = slv.check_sat();

    l1->print(mod);
    l2->print(mod);
}
