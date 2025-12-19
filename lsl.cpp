#include <exprtk.hpp>
#include <iostream>
#include<vector>

#include "antlr4-runtime.h"
#include"z3++.h"
#include "src/leaf_spine/demux_switch.hpp"
#include "src/leaf_spine/leaf_sts.hpp"
#include "src/leaf_spine/leaf_utils.hpp"
#include "src/utils.hpp"
#include "src/gen/constr_extractor.hpp"
#include "src/gen/wl_parser.hpp"

using namespace std;
using namespace z3;
using namespace antlr4;
using namespace chrono;

constexpr int MAX_ENQ = 5;
constexpr int MAX_DEQ = 1;
constexpr int TIME_STEPS = 10;
constexpr int OFFSET = 3;
constexpr int PKT_TYPES = 12;
constexpr int RANDOM_SEED = 6000;


expr query_val(SmtSolver &slv, ev3 &path_C) {
    expr s = slv.s.ctx().int_val(0);
    // int t = TIME_STEPS - 1 - OFFSET;
    int t = TIME_STEPS - 1;
    for (int i = 0; i < path_C.size(); ++i) {
        s = s + sum(path_C[i][t]);
    }
    return s;
}

expr query(SmtSolver &slv, ev3 &path_C) {
    int thresh = 10;
    cout << "QUERY THRESHOLD:" << thresh << endl;
    expr q = query_val(slv, path_C) >= thresh;
    return q;
}

int check_wl(vector<string> wl, bool sat, int buff_cap) {
    ev3 I;

    map<int, int> pkt_type_to_dst = {
        {0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5},
        {6, 0}, {7, 1}, {8, 2}, {9, 3}, {10, 4}, {11, 5}
    };
    map<int, int> pkt_type_to_ecmp = {
        {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0},
        {6, 1}, {7, 1}, {8, 1}, {9, 1}, {10, 1}, {11, 1}
    };

    SmtSolver slv(RANDOM_SEED);


    ev3 tmp_I = slv.ivvv(6, TIME_STEPS, PKT_TYPES, "TMP_I");
    slv.s.push();
    auto vals_map = add_workload(slv, tmp_I, TIME_STEPS, pkt_type_to_dst, pkt_type_to_ecmp, wl);
    slv.s.pop();

    set<int> used_dsts = get_used_vals(vals_map, "dst");
    set<int> used_ecmps = get_used_vals(vals_map, "ecmp");
    cout << "Used DST:" << " ";
    printSet(used_dsts);
    cout << endl;
    cout << "Used ECMP:" << " ";
    printSet(used_ecmps);
    cout << endl;
    set<int> zero_inputs = get_zero_inputs(vals_map);
    cout << "Zero Inputs:" << " ";
    printSet(zero_inputs);
    cout << endl;

    // set<int> used_dsts = {3, 5};
    // set<int> used_ecmps = {0, 1};


    LeafBase *l1;
    map<tuple<int, int>, vector<int> > l1_ports = {
        {{0, 1}, {}},
        {{0, 2}, {}},
        {{0, 3}, {}},
        {{1, 0}, {}},
        {{1, 2}, {}},
        {{1, 3}, {}},
        {{2, 0}, {}},
        {{2, 1}, {}},
        {{3, 0}, {}},
        {{3, 1}, {}}
    };

    vector l1_pkt_type_to_nxt_hop = {0, 1, 2, 2, 2, 2, 0, 1, 3, 3, 3, 3};
    vector port_to_ecmp = {-1, -1, 0, 1};
    vector l1_src_port_to_input = {0, 1, -1, -1};
    fix(l1_ports, l1_pkt_type_to_nxt_hop, used_dsts, used_ecmps, pkt_type_to_dst, pkt_type_to_ecmp, port_to_ecmp,
        l1_src_port_to_input, zero_inputs);
    update_ports({vals_map[0], vals_map[1]}, l1_ports, l1_src_port_to_input, l1_pkt_type_to_nxt_hop, port_to_ecmp);
    cout << "L1 Ports:" << endl;
    printPorts(l1_ports);
    l1 = new DemuxSwitch(slv, "l1", l1_ports, TIME_STEPS, PKT_TYPES, buff_cap, MAX_ENQ, MAX_DEQ,
                         l1_pkt_type_to_nxt_hop
    );


    LeafBase *l2;
    map<tuple<int, int>, vector<int> > l2_ports = {
        {{0, 1}, {}},
        {{0, 2}, {}},
        {{0, 3}, {}},
        {{1, 0}, {}},
        {{1, 2}, {}},
        {{1, 3}, {}},
        {{2, 0}, {}},
        {{2, 1}, {}},
        {{3, 0}, {}},
        {{3, 1}, {}}
    };

    vector l2_pkt_type_to_nxt_hop = {2, 2, 0, 1, 2, 2, 3, 3, 0, 1, 3, 3};
    vector l2_src_port_to_input = {2, 3, -1, -1};
    fix(l2_ports, l2_pkt_type_to_nxt_hop, used_dsts, used_ecmps, pkt_type_to_dst, pkt_type_to_ecmp, port_to_ecmp,
        l2_src_port_to_input, zero_inputs);
    update_ports({vals_map[2], vals_map[3]}, l2_ports, l2_src_port_to_input, l2_pkt_type_to_nxt_hop, port_to_ecmp);
    cout << "L2 Ports:" << endl;
    printPorts(l2_ports);
    l2 = new DemuxSwitch(slv, "l2", l2_ports, TIME_STEPS, PKT_TYPES, buff_cap, MAX_ENQ, MAX_DEQ,
                         l2_pkt_type_to_nxt_hop
    );


    LeafBase *l3;
    map<tuple<int, int>, vector<int> > l3_ports = {
        {{0, 1}, {}},
        {{0, 2}, {}},
        {{0, 3}, {}},
        {{1, 0}, {}},
        {{1, 2}, {}},
        {{1, 3}, {}},
        {{2, 0}, {}},
        {{2, 1}, {}},
        {{3, 0}, {}},
        {{3, 1}, {}}
    };

    vector l3_pkt_type_to_nxt_hop = {2, 2, 2, 2, 0, 1, 3, 3, 3, 3, 0, 1};
    vector l3_src_port_to_input = {4, 5, -1, -1};
    fix(l3_ports, l3_pkt_type_to_nxt_hop, used_dsts, used_ecmps, pkt_type_to_dst, pkt_type_to_ecmp, port_to_ecmp,
        l3_src_port_to_input, zero_inputs);
    update_ports({vals_map[4], vals_map[5]}, l3_ports, l3_src_port_to_input, l3_pkt_type_to_nxt_hop, port_to_ecmp);
    cout << "L3 Ports:" << endl;
    printPorts(l3_ports);

    l3 = new DemuxSwitch(slv, "l3", l3_ports, TIME_STEPS, PKT_TYPES, buff_cap, MAX_ENQ, MAX_DEQ,
                         l3_pkt_type_to_nxt_hop
    );

    // exit(0);
    FperfLeafSts *s1;
    map<tuple<int, int>, vector<int> > s1_ports = {
        {{0, 1}, {2, 3}},
        {{0, 2}, {4, 5}},
        {{1, 0}, {0, 1}},
        {{1, 2}, {4, 5}},
        {{2, 0}, {0, 1}},
        {{2, 1}, {2, 3}}
    };

    vector s1_pkt_type_to_nxt_hop = {0, 0, 1, 1, 2, 2, 0, 0, 1, 1, 2, 2};
    s1 = new DemuxSwitch(slv, "s1", s1_ports, TIME_STEPS, PKT_TYPES, buff_cap, MAX_ENQ, MAX_DEQ,
                         s1_pkt_type_to_nxt_hop);

    LeafBase *s2;
    map<tuple<int, int>, vector<int> > s2_ports = {
        {{0, 1}, {8, 9}},
        {{0, 2}, {10, 11}},
        {{1, 0}, {6, 7}},
        {{1, 2}, {10, 11}},
        {{2, 0}, {6, 7}},
        {{2, 1}, {8, 9}}
    };

    vector s2_pkt_type_to_nxt_hop = {0, 0, 1, 1, 2, 2, 0, 0, 1, 1, 2, 2};
    s2 = new DemuxSwitch(slv, "s2", s2_ports, TIME_STEPS, PKT_TYPES, buff_cap, MAX_ENQ, MAX_DEQ,
                         s2_pkt_type_to_nxt_hop
    );


    auto base_l1 = l1->base_constrs();
    // auto base_l1_merged = merge(base_l1, slv.ctx, "base_l1");
    slv.add(base_l1);

    auto base_l2 = l2->base_constrs();
    auto base_l2_merged = merge(base_l2, slv.ctx, "base_l2");
    slv.add(base_l2_merged);

    auto base_l3 = l3->base_constrs();
    auto base_l3_merged = merge(base_l3, slv.ctx, "base_l3");
    slv.add(base_l3_merged);

    auto base_s1 = s1->base_constrs();
    auto base_s1_merged = merge(base_s1, slv.ctx, "base_s1");
    slv.add(base_s1_merged);

    auto base_s2 = s2->base_constrs();
    auto base_s2_merged = merge(base_s2, slv.ctx, "base_s2");
    slv.add(base_s2_merged);

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

    I.push_back(l1->get_in_port(0));
    I.push_back(l1->get_in_port(1));
    I.push_back(l2->get_in_port(0));
    I.push_back(l2->get_in_port(1));
    I.push_back(l3->get_in_port(0));
    I.push_back(l3->get_in_port(1));

    ev3 path_C;
    path_C.push_back(l1->buffs[{0, 2}]->getExpandedC());
    path_C.push_back(s1->buffs[{0, 2}]->getExpandedC());
    path_C.push_back(l3->buffs[{2, 1}]->getExpandedC());

    ev3 O;
    O.push_back(l1->get_out_port(0));
    O.push_back(l1->get_out_port(1));
    O.push_back(l2->get_out_port(0));
    O.push_back(l2->get_out_port(1));
    O.push_back(l3->get_out_port(0));
    O.push_back(l3->get_out_port(1));

    map<int, vector<int> > dst_to_pkt_type;
    for (auto &[pkt_type,dst]: pkt_type_to_dst)
        dst_to_pkt_type[dst].push_back(pkt_type);

    map<int, vector<int> > ecmp_to_pkt_type;
    for (auto &[pkt_type,ecmp]: pkt_type_to_ecmp)
        ecmp_to_pkt_type[ecmp].push_back(pkt_type);

    slv.s.push();
    add_workload(slv, I, TIME_STEPS, pkt_type_to_dst, pkt_type_to_ecmp, wl);

    bool debug = false;

    auto start_t = high_resolution_clock::now();

    if (sat) {
        cout << "Checking WL & !Q is SAT" << endl;
        slv.s.push();
        slv.add(NamedExp(query(slv, path_C), "query").negate());
        slv.check_sat();
        slv.s.pop();
        cout << "SAT: WL & !Q is SAT" << endl;
    }
    if (!sat && debug) {
        slv.s.push();
        slv.add(NamedExp(query(slv, O), "query"));
        slv.check_sat();
        slv.s.pop();
        cout << "UNSAT: WL & Q is SAT" << endl;
    }
    if (!sat) {
        cout << "Checking WL & !Q is UNSAT" << endl;
        slv.s.push();
        slv.add(NamedExp(query(slv, path_C), "query").negate());
        slv.check_unsat();
        slv.s.pop();
        cout << "UNSAT: WL & !Q is UNSAT" << endl;
    }

    auto end_t = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_t - start_t);
    long result = duration.count();
    return result;
}

int main(const int argc, const char *argv[]) {
    if (argc < 2)
        return 1;
    int buff_cap = atoi(argv[1]);
    string wl_file_path = format("./data/sub_wls/lsl/lsl.{}.txt", buff_cap);
    vector<vector<string> > wls = read_wl_file(wl_file_path);
    string out_file_path = format("./data/logs/lsl/lsl.{}.txt", buff_cap);
    ofstream out(out_file_path, ios::out);
    out << "scheduler, buf_size, wl_idx, time_millis, solver_res" << endl;
    for (int i = 0; i < wls.size(); ++i) {
        auto wl = wls[i];
        string res_stat = wl[0];
        cout << "WL: " << i + 1 << "/" << wls.size() << " " << res_stat << endl;
        if (res_stat == "SKIP") {
            cout << "SKIPPING WORKLOAD!!!!!!!!!!" << i << endl;
            continue;
        }
        bool sat = res_stat == "SAT";
        wl.emplace_back("[1, 10]: cenq(0, t) <= t");
        wl.emplace_back("[1, 10]: dst(0, t)  == 5");
        wl.emplace_back("[1, 10]: ecmp(0, t) == 0");
        int res = check_wl(wl, sat, buff_cap);
        out << "leaf" << "," << buff_cap << ", " << i << ", " << res << ", " << res_stat << endl;
    }
}
