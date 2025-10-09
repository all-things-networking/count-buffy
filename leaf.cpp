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

    LeafSts *l2;
    vector<tuple<int, int> > l2_ports = {
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
    vector l2_pkt_type_to_nxt_hop = {2, 2, 0, 1, 2, 2, 3, 3, 0, 1, 3, 3};
    l2 = new DemuxSwitch(slv, "l2", l2_ports, TIMESTEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ,
                         l2_pkt_type_to_nxt_hop
    );
    I.push_back(l2->get_in_port(0));
    I.push_back(l2->get_in_port(1));

    LeafSts *l3;
    vector<tuple<int, int> > l3_ports = {
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
    vector l3_pkt_type_to_nxt_hop = {2, 2, 2, 2, 0, 1, 3, 3, 3, 3, 0, 1};
    l3 = new DemuxSwitch(slv, "l3", l3_ports, TIMESTEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ,
                         l3_pkt_type_to_nxt_hop
    );
    I.push_back(l3->get_in_port(0));
    I.push_back(l3->get_in_port(1));

    LeafSts *s1;
    vector<tuple<int, int> > s1_ports = {
        {0, 1},
        {0, 2},
        {1, 0},
        {1, 2},
        {2, 0},
        {2, 1}
    };
    vector s1_pkt_type_to_nxt_hop = {0, 0, 1, 1, 2, 2, 0, 0, 1, 1, 2, 2};
    s1 = new DemuxSwitch(slv, "s1", s1_ports, TIMESTEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ,
                         s1_pkt_type_to_nxt_hop
    );

    LeafSts *s2;
    vector<tuple<int, int> > s2_ports = {
        {0, 1},
        {0, 2},
        {1, 0},
        {1, 2},
        {2, 0},
        {2, 1}
    };
    vector s2_pkt_type_to_nxt_hop = {0, 0, 1, 1, 2, 2, 0, 0, 1, 1, 2, 2};
    s2 = new DemuxSwitch(slv, "s2", s2_ports, TIMESTEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ,
                         s2_pkt_type_to_nxt_hop
    );

    slv.add(link_ports(l1->get_out_port(2), s1->get_in_port(0)), "l1-s1");
    slv.add(link_ports(l2->get_out_port(2), s1->get_in_port(1)), "l2-s1");
    slv.add(link_ports(l3->get_out_port(2), s1->get_in_port(2)), "l3-s1");
    slv.add(link_ports(s1->get_out_port(0), l1->get_in_port(2)), "s1-l1");
    slv.add(link_ports(s1->get_out_port(1), l2->get_in_port(2)), "s1-l2");
    slv.add(link_ports(s1->get_out_port(2), l3->get_in_port(2)), "s1-l3");

    slv.add(link_ports(l1->get_out_port(3), s2->get_in_port(0)), "l1-s2");
    slv.add(link_ports(l2->get_out_port(3), s2->get_in_port(1)), "l2-s2");
    slv.add(link_ports(l3->get_out_port(3), s2->get_in_port(2)), "l3-s2");
    slv.add(link_ports(s2->get_out_port(0), l1->get_in_port(3)), "s2-l1");
    slv.add(link_ports(s2->get_out_port(1), l2->get_in_port(3)), "s2-l2");
    slv.add(link_ports(s2->get_out_port(2), l3->get_in_port(3)), "s2-l3");

    // in_port, time, type -> count
    // map<tuple<int, int, int>, int> ins_l1 = {
        // {{0, 0, 10}, 2},
    // };
    // auto constr_l1 = add_constr(l1, ins_l1, {0, 1});
    // slv.add({constr_l1, "l1-inp"});

    // map<tuple<int, int, int>, int> ins_l2 = {
    // };
    // auto constr_l2 = add_constr(l2, ins_l2, {0, 1});
    // slv.add({constr_l2, "l2-inp"});

    // map<tuple<int, int, int>, int> ins_l3 = {
    // };
    // auto constr_l3 = add_constr(l3, ins_l3, {0, 1});
    // slv.add({constr_l3, "l3-inp"});

    auto base_l1 = l1->base_constrs();
    auto base_l1_merged = merge(base_l1, "base_l1");

    auto base_l2 = l2->base_constrs();
    auto base_l2_merged = merge(base_l2, "base_l2");

    auto base_l3 = l3->base_constrs();
    auto base_l3_merged = merge(base_l3, "base_l3");

    auto base_s1 = s1->base_constrs();
    auto base_s1_merged = merge(base_s1, "base_s1");

    auto base_s2 = s2->base_constrs();
    auto base_s2_merged = merge(base_s2, "base_s2");

    slv.add(base_l1_merged);
    slv.add(base_l2_merged);
    slv.add(base_l3_merged);
    slv.add(base_s1_merged);
    slv.add(base_s2_merged);

    add_workload(slv, I, num_spines, num_leafs, host_per_leaf, TIMESTEPS);
    //
    auto mod = slv.check_sat();
    //
    l1->print(mod);

    l2->print(mod);

    l3->print(mod);

    s1->print(mod);

    s2->print(mod);
}
