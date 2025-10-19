#include <iostream>
#include<vector>
#include "src/leaf_utils.hpp"

#include "antlr4-runtime.h"
#include"z3++.h"
#include "src/DemuxSwitch.hpp"
#include "src/leaf_sts.hpp"
#include "src/prio_sts.hpp"
#include "src/gen/constr_extractor.hpp"
#include "src/gen/wl_parser.hpp"

class fperfVisitor;
using namespace std;
using namespace z3;
using namespace antlr4;

constexpr int MAX_ENQ = 4;
constexpr int MAX_DEQ = 1;
constexpr int TIMESTEPS = 10;
constexpr int NUM_PORTS = 3;
constexpr int PKT_TYPES = 12;
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

expr merge_ports(vector<ev2> out_ports, ev2 in) {
    auto e = out_ports[0][0][0].ctx().bool_val(true);
    for (int t = 0; t < out_ports[0].size(); ++t) {
        auto out = out_ports[0][t];
        for (int i = 1; i < out_ports.size(); ++i) {
            out = out + out_ports[i][t];
        }
        e = e && (out == in[t]);
    }
    return e;
}

expr add_constr(LeafSts *sts, map<tuple<int, int, int>, int> inp, set<int> in_ports) {
    expr e = sts->slv.ctx.bool_val(true);
    for (int port: in_ports) {
        auto in_port = sts->get_in_port(port);
        for (int t = 0; t < TIMESTEPS; ++t) {
            for (int k = 0; k < PKT_TYPES; ++k) {
                int val = 0;
                if (inp.contains({port, t, k})) {
                    val = inp[{port, t, k}];
                    cout << "INCLUDES: " << port << "@" << t << " -> " << k << endl;
                }
                e = e && in_port[t][k] == val;
            }
        }
    }
    return e;
}


int main(const int argc, const char *argv[]) {
    SmtSolver slv;
    int num_spines = 2;
    int num_leafs = 3;
    int host_per_leaf = 2;
    int num_in_bufs = num_leafs * host_per_leaf;
    auto m = slv.check_sat();
    ev3 I;
    // = slv.ivvv(num_in_bufs, TIMESTEPS, num_in_bufs * num_spines, format("III"));
    // exit(0);


    LeafSts *l1;
    vector<tuple<int, int> > l1_ports = {
        {0, 1},
        {0, 2},
        {0, 3},
        {1, 0},
        {1, 2},
        {1, 3},
        {2, 0},
        {2, 1},
        {3, 0},
        {3, 1}
    };
    vector l1_pkt_type_to_nxt_hop = {0, 1, 2, 2, 2, 2, 0, 1, 3, 3, 3, 3};
    l1 = new DemuxSwitch(slv, "l1", l1_ports, TIMESTEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ,
                         l1_pkt_type_to_nxt_hop
    );
    I.push_back(l1->get_in_port(0));
    I.push_back(l1->get_in_port(1));


    auto base_l1 = l1->base_constrs();
    auto base_l1_merged = merge(base_l1, "base_l1");

    // add_workload(slv, I, num_spines, num_leafs, host_per_leaf, TIMESTEPS);

    slv.add(base_l1_merged);

    auto mod = slv.check_sat();
    // l1->print(mod);

    cout << str(I, mod).str() << endl;
}
