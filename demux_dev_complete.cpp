#include <iostream>
#include<vector>

#include "antlr4-runtime.h"
#include"z3++.h"
#include "src/DemuxSwitch.hpp"
#include "src/leaf_sts.hpp"
#include "src/leaf_utils.hpp"
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
using namespace chrono;

constexpr int MAX_ENQ = 4;
constexpr int MAX_DEQ = 1;
constexpr int TIME_STEPS = 10;
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

expr add_constr(LeafSts *sts, map<tuple<int, int, int>, int> inp) {
    expr e = sts->slv.ctx.bool_val(true);
    auto in_ports = sts->get_in_ports();
    for (int port: in_ports) {
        auto in_port = sts->get_in_port(port);
        for (int t = 0; t < TIME_STEPS; ++t) {
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

expr query(SmtSolver &slv, ev3 &O) {
    expr s = slv.s.ctx().int_val(0);
    int i = 1;
    for (int t = 0; t < TIME_STEPS; ++t)
        s = s + sum(O[i][t]);
    expr q = s <= TIME_STEPS / 2;
    return q;
}

int main(const int argc, const char *argv[]) {
    map<int, int> pkt_type_to_dst = {
        {0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5},
        {6, 0}, {7, 1}, {8, 2}, {9, 3}, {10, 4}, {11, 5}
    };
    map<int, int> pkt_type_to_ecmp = {
        {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0},
        {6, 1}, {7, 1}, {8, 1}, {9, 1}, {10, 1}, {11, 1}
    };

    SmtSolver slv;
    LeafSts *l1;
    map<tuple<int, int>, vector<int> > l1_ports = {
        {{0, 1}, {}},
        {{0, 2}, {5}},
        {{0, 3}, {}},
        {{1, 2}, {}},
        {{1, 3}, {9}},
        {{1, 0}, {}},
        {{2, 0}, {}},
        {{2, 1}, {}},
        {{3, 0}, {6}},
        {{3, 1}, {7}}
    };

    vector l1_pkt_type_to_nxt_hop = {0, 1, 2, 2, 2, 2, 0, 1, 3, 3, 3, 3};
    l1 = new DemuxSwitch(slv, "l1", l1_ports, TIME_STEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ,
                         l1_pkt_type_to_nxt_hop
    );

    LeafSts *l2;
    map<tuple<int, int>, vector<int> > l2_ports = {
        {{0, 1}, {}},
        {{0, 2}, {4}},
        {{0, 3}, {}},
        {{1, 0}, {}},
        {{1, 2}, {}},
        {{1, 3}, {}},
        {{2, 0}, {}},
        {{2, 1}, {}},
        {{3, 0}, {}},
        {{3, 1}, {9}}
    };
    vector l2_pkt_type_to_nxt_hop = {2, 2, 0, 1, 2, 2, 3, 3, 0, 1, 3, 3};
    l2 = new DemuxSwitch(slv, "l2", l2_ports, TIME_STEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ,
                         l2_pkt_type_to_nxt_hop
    );

    LeafSts *l3;
    map<tuple<int, int>, vector<int> > l3_ports = {
        {{0, 1}, {}},
        {{0, 2}, {}},
        {{0, 3}, {6}},
        {{1, 0}, {}},
        {{1, 2}, {}},
        {{1, 3}, {7}},
        {{2, 0}, {4}},
        {{2, 1}, {5}},
        {{3, 0}, {}},
        {{3, 1}, {}}
    };
    vector l3_pkt_type_to_nxt_hop = {2, 2, 2, 2, 0, 1, 3, 3, 3, 3, 0, 1};
    l3 = new DemuxSwitch(slv, "l3", l3_ports, TIME_STEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ,
                         l3_pkt_type_to_nxt_hop
    );


    LeafSts *s1;
    map<tuple<int, int>, vector<int> > s1_ports = {
        {{0, 1}, {2, 3}},
        {{0, 2}, {4, 5}},
        {{1, 0}, {0, 1,}},
        {{1, 2}, {4, 5}},
        {{2, 0}, {0, 1,}},
        {{2, 1}, {2, 3}}
    };
    vector s1_pkt_type_to_nxt_hop = {0, 0, 1, 1, 2, 2, 0, 0, 1, 1, 2, 2};
    s1 = new DemuxSwitch(slv, "s1", s1_ports, TIME_STEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ,
                         s1_pkt_type_to_nxt_hop
    );

    LeafSts *s2;
    map<tuple<int, int>, vector<int> > s2_ports = {
        {{0, 1}, {8, 9}},
        {{0, 2}, {10, 11}},
        {{1, 0}, {6, 7}},
        {{1, 2}, {10, 11}},
        {{2, 0}, {6, 7}},
        {{2, 1}, {8, 9}}
    };
    vector s2_pkt_type_to_nxt_hop = {0, 0, 1, 1, 2, 2, 0, 0, 1, 1, 2, 2};
    s2 = new DemuxSwitch(slv, "s2", s2_ports, TIME_STEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ,
                         s2_pkt_type_to_nxt_hop
    );

    auto base_s1 = s1->base_constrs();
    auto base_s1_merged = merge(base_s1, "base_s1");
    slv.add(base_s1_merged);

    auto base_s2 = s2->base_constrs();
    auto base_s2_merged = merge(base_s2, "base_s2");
    slv.add(base_s2_merged);

    auto base_l1 = l1->base_constrs();
    auto base_l1_merged = merge(base_l1, "base_l1");
    slv.add(base_l1_merged);

    auto base_l2 = l2->base_constrs();
    auto base_l2_merged = merge(base_l2, "base_l2");
    slv.add(base_l2_merged);

    auto base_l3 = l3->base_constrs();
    auto base_l3_merged = merge(base_l3, "base_l3");
    slv.add(base_l3_merged);

    slv.add({link_ports(l1->get_out_port(2), s1->get_in_port(0)), format("Link: {} -> {}", "l1_2", "s1_0")});
    slv.add({link_ports(l1->get_out_port(3), s2->get_in_port(0)), format("Link: {} -> {}", "l1_3", "s2_0")});
    slv.add({link_ports(s1->get_out_port(0), l1->get_in_port(2)), format("Link: {} -> {}", "s1_0", "l1_2")});
    slv.add({link_ports(s2->get_out_port(0), l1->get_in_port(3)), format("Link: {} -> {}", "s2_0", "l1_3")});

    slv.add({link_ports(l2->get_out_port(2), s1->get_in_port(1)), format("Link: {} -> {}", "l2_2", "s1_0")});
    slv.add({link_ports(l2->get_out_port(3), s2->get_in_port(1)), format("Link: {} -> {}", "l2_3", "s2_0")});
    slv.add({link_ports(s1->get_out_port(1), l2->get_in_port(2)), format("Link: {} -> {}", "s1_0", "l2_2")});
    slv.add({link_ports(s2->get_out_port(1), l2->get_in_port(3)), format("Link: {} -> {}", "s2_0", "l2_3")});

    slv.add({link_ports(l3->get_out_port(2), s1->get_in_port(2)), format("Link: {} -> {}", "l3_2", "s1_0")});
    slv.add({link_ports(l3->get_out_port(3), s2->get_in_port(2)), format("Link: {} -> {}", "l3_3", "s2_0")});
    slv.add({link_ports(s1->get_out_port(2), l3->get_in_port(2)), format("Link: {} -> {}", "s1_0", "l3_2")});
    slv.add({link_ports(s2->get_out_port(2), l3->get_in_port(3)), format("Link: {} -> {}", "s2_0", "l3_3")});


    // slv.add({link_ports(s1->get_out_port(1), l3->get_in_port(2)), format("Link: {} -> {}", "s1_1", "l3_2")});
    // slv.add({link_ports(s2->get_out_port(1), l3->get_in_port(3)), format("Link: {} -> {}", "s2_1", "l3_3")});

    ev3 I;
    I.push_back(l1->get_in_port(0));
    I.push_back(l1->get_in_port(1));
    I.push_back(l2->get_in_port(0));
    I.push_back(l2->get_in_port(1));

    ev3 O;
    O.push_back(l3->get_out_port(0));
    O.push_back(l3->get_out_port(1));

    map<int, vector<int> > dst_to_pkt_type;
    for (auto &[pkt_type,dst]: pkt_type_to_dst)
        dst_to_pkt_type[dst].push_back(pkt_type);

    map<int, vector<int> > ecmp_to_pkt_type;
    for (auto &[pkt_type,ecmp]: pkt_type_to_ecmp)
        ecmp_to_pkt_type[ecmp].push_back(pkt_type);

    add_workload(slv, I, TIME_STEPS, pkt_type_to_dst, pkt_type_to_ecmp);

    if (false) {
        expr_vector v(slv.ctx);
        for (int i = 0; i < I.size(); ++i) {
            for (int t = 0; t < I[0].size(); ++t) {
                expr e = dst_val(I, slv, dst_to_pkt_type, i, t);
                v.push_back((e == 2 || e == 3 || e == -1));
                e = ecmp_val(I, slv, ecmp_to_pkt_type, i, t);
                v.push_back((e == 0 || e == -1));
            }
        }
        slv.add(mk_and(v));
    }


    if (1) {
        slv.s.push();
        slv.add(NamedExp(query(slv, O), "query").negate());
        auto start_t = high_resolution_clock::now();
        slv.check_unsat();
        auto end_t = high_resolution_clock::now();
        auto unsat_duration = duration_cast<milliseconds>(end_t - start_t);
        slv.s.pop();
        cout << "UNSAT VTIME: " << unsat_duration.count() << endl;
        exit(0);
    }

    slv.s.push();
    // slv.add({query(slv, O), "query"});

    auto start_t = high_resolution_clock::now();
    auto mod = slv.check_sat();
    auto end_t = high_resolution_clock::now();
    auto sat_duration = duration_cast<milliseconds>(end_t - start_t);

    cout << "Valid" << endl;
    for (int i = 0; i < I.size(); ++i) {
        for (int t = 0; t < I[0].size(); ++t) {
            expr e = valid_meta(I, slv, i, t);
            cout << mod.eval(e) << ", ";
        }
        cout << endl;
    }

    cout << "DST" << endl;
    for (int i = 0; i < I.size(); ++i) {
        for (int t = 0; t < I[0].size(); ++t) {
            expr e = dst_val(I, slv, dst_to_pkt_type, i, t);
            cout << mod.eval(e) << ", ";
        }
        cout << endl;
    }

    cout << "ECMP" << endl;
    for (int i = 0; i < I.size(); ++i) {
        for (int t = 0; t < I[0].size(); ++t) {
            expr e = ecmp_val(I, slv, ecmp_to_pkt_type, i, t);
            cout << mod.eval(e) << ", ";
        }
        cout << endl;
    }

    cout << "Input" << endl;
    cout << str(I, mod).str() << endl << endl;

    cout << "Output" << endl;
    cout << str(O, mod).str() << endl << endl;

    // cout << "l1" << endl << "##################################" << endl;
    // l1->print(mod);
    //
    // cout << "s1" << endl << "##################################" << endl;
    // s1->print(mod);
    //
    cout << "l3" << endl << "##################################" << endl;
    l3->print(mod);

    cout << "SAT VTIME: " << sat_duration.count() << endl;
    slv.s.pop();
}
