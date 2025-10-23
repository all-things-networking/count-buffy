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
        {{0, 2}, {5, 11}},
        {{1, 2}, {}},
        {{1, 0}, {}},
        {{2, 0}, {}},
        {{2, 1}, {}},
    };

    // vector<tuple<int, int> > l1_ports = {
    //     {0, 2},
    //     {1, 2},
    //     {2, 0},
    //     {2, 1}
    // };

    vector l1_pkt_type_to_nxt_hop = {0, 1, 2, 2, 2, 2, 0, 1, 2, 2, 2, 2};
    l1 = new DemuxSwitch(slv, "l1", l1_ports, TIME_STEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ,
                         l1_pkt_type_to_nxt_hop
    );

    LeafSts *l2;
    map<tuple<int, int>, vector<int> > l2_ports = {
        {{0, 1}, {}},
        {{2, 0}, {}},
        {{2, 1}, {}},
        {{1, 0}, {}},
        {{0, 2}, {}},
        {{1, 2}, {}}
    };
    vector l2_pkt_type_to_nxt_hop = {2, 2, 0, 1, 2, 2, 2, 2, 0, 1, 2, 2};
    l2 = new DemuxSwitch(slv, "l2", l2_ports, TIME_STEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ,
                         l2_pkt_type_to_nxt_hop
    );

    LeafSts *l3;
    map<tuple<int, int>, vector<int> > l3_ports = {
        {{0, 1}, {}},
        {{2, 0}, {}},
        {{2, 1}, {5, 11}},
        {{1, 0}, {}},
        {{0, 2}, {}},
        {{1, 2}, {}}
    };
    vector l3_pkt_type_to_nxt_hop = {2, 2, 2, 2, 0, 1, 2, 2, 2, 2, 0, 1};
    l3 = new DemuxSwitch(slv, "l3", l3_ports, TIME_STEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ,
                         l3_pkt_type_to_nxt_hop
    );

    LeafSts *s1;
    map<tuple<int, int>, vector<int> > s1_ports = {
        {{0, 1}, {0, 1, 2, 3, 4, 5}}
    };
    vector s1_pkt_type_to_nxt_hop = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    s1 = new DemuxSwitch(slv, "s1", s1_ports, TIME_STEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ,
                         s1_pkt_type_to_nxt_hop
    );

    auto base1 = l1->base_constrs();
    auto base1_merged = merge(base1, "base1");
    slv.add(base1_merged);

    auto base2 = s1->base_constrs();
    auto base2_merged = merge(base2, "base2");
    slv.add(base2_merged);

    auto base3 = l2->base_constrs();
    auto base3_merged = merge(base3, "base3");
    slv.add(base3_merged);


    slv.add({link_ports(l1->get_out_port(2), s1->get_in_port(0)), format("Link: {} -> {}", "l1_2", "s1_0")});
    slv.add({link_ports(s1->get_out_port(1), l3->get_in_port(2)), format("Link: {} -> {}", "s1_1", "l3_2")});
    ev3 I;
    I.push_back(l1->get_in_port(0));
    I.push_back(l1->get_in_port(1));

    ev3 O;
    O.push_back(l2->get_out_port(0));
    O.push_back(l2->get_out_port(1));

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


    slv.s.push();
    slv.add(NamedExp(query(slv, O), "query").negate());
    auto start_t = high_resolution_clock::now();
    slv.check_unsat();
    auto end_t = high_resolution_clock::now();
    auto unsat_duration = duration_cast<milliseconds>(end_t - start_t);
    slv.s.pop();

    slv.s.push();
    slv.add({query(slv, O), "query"});

    start_t = high_resolution_clock::now();
    auto mod = slv.check_sat();
    end_t = high_resolution_clock::now();
    auto sat_duration = duration_cast<milliseconds>(end_t - start_t);

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

    cout << "l1" << endl << "##################################" << endl;
    l1->print(mod);

    cout << "s1" << endl << "##################################" << endl;
    s1->print(mod);

    cout << "l2" << endl << "##################################" << endl;
    l2->print(mod);

    cout << "UNSAT VTIME: " << unsat_duration.count() << endl;
    cout << "SAT VTIME: " << sat_duration.count() << endl;
    slv.s.pop();
}
