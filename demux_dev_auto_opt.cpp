#include <exprtk.hpp>
#include <iostream>
#include<vector>

#include "antlr4-runtime.h"
#include"z3++.h"
#include "src/DemuxSwitch.hpp"
#include "src/leaf_sts.hpp"
#include "src/leaf_utils.hpp"
#include "src/prio_sts.hpp"
#include "src/utils.hpp"
#include "src/gen/constr_extractor.hpp"
#include "src/gen/wl_parser.hpp"

class fperfVisitor;
using namespace std;
using namespace z3;
using namespace antlr4;
using namespace chrono;

constexpr int MAX_ENQ = 5;
constexpr int MAX_DEQ = 1;
constexpr int TIME_STEPS = 10;
constexpr int PKT_TYPES = 12;
constexpr int BUFF_CAP = 30;

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

expr add_constr(LeafBase *sts, map<tuple<int, int, int>, int> inp) {
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
    int i = 5;
    for (int t = 0; t < TIME_STEPS - 3; ++t)
        s = s + sum(O[i][t]);
    int thresh = TIME_STEPS / 2;
    cout << "QUERY THRESHOLD:" << thresh << endl;
    expr q = s < thresh;
    return q;
}

void printPorts(const map<tuple<int, int>, vector<int> > &ports) {
    for (const auto &[key, values]: ports) {
        auto [a, b] = key;
        cout << "(" << a << ", " << b << "): ";
        for (int v: values) cout << v << " ";
        cout << "\n";
    }
}

void fix(map<tuple<int, int>, vector<int> > &ports,
         vector<int> pkt_type_to_nxt_hop,
         set<int> used_dsts,
         set<int> used_ecmps,
         map<int, int> pkt_type_to_dst,
         map<int, int> pkt_type_to_ecmp,
         vector<int> port_to_ecmp,
         vector<int> src_port_to_input,
         set<int> zero_inputs
) {
    for (auto &[src_dst, p]: ports) {
        int src_port = get<0>(src_dst);
        int dst_port = get<1>(src_dst);
        if (src_port < 2)
            continue;
        for (int k = 0; k < PKT_TYPES; ++k) {
            int ecmp_value = pkt_type_to_ecmp[k];
            int dst_value = pkt_type_to_dst[k];
            if (used_dsts.contains(dst_value) && used_ecmps.contains(ecmp_value) && (pkt_type_to_nxt_hop[k] == dst_port)
                && (port_to_ecmp[src_port] == ecmp_value) && (!zero_inputs.contains(
                    src_port_to_input[src_port]))) {
                if (find(p.begin(), p.end(), k) == p.end()) {
                    p.push_back(k);
                }
            }
        }
    }
}

void update_ports(vector<map<string, set<int> > > vals_per_input,
                  map<tuple<int, int>, vector<int> > &ports,
                  vector<int> port_to_input,
                  vector<int> pkt_type_to_next_hop,
                  vector<int> port_to_ecmp
) {
    for (auto &[src_dst, pkt_types]: ports) {
        int src_port = get<0>(src_dst);
        int dst_port = get<1>(src_dst);
        int src_input = port_to_input[src_port];
        int src_input_idx = src_input % vals_per_input.size();
        int dst_input = port_to_input[dst_port];
        if (src_input == -1)
            continue;

        set<int> used_dst_vals = vals_per_input[src_input_idx]["dst"];
        set<int> used_ecmps = vals_per_input[src_input_idx]["ecmp"];

        // int dst_ecmp = dst_port % 2;
        int dst_ecmp = port_to_ecmp[dst_port];

        vector<int> dst_ecmps;
        if (dst_ecmp != -1)
            dst_ecmps.push_back(dst_ecmp);
        else
            for (auto ecmp: used_ecmps)
                dst_ecmps.push_back(ecmp);

        for (auto dst_ecmp_val: dst_ecmps) {
            if (!used_ecmps.contains(dst_ecmp_val))
                continue;

            int ecmp_offset = dst_ecmp_val * 6;
            for (auto used_dst_val: used_dst_vals) {
                int pkt_type = ecmp_offset + used_dst_val;
                int nxt_hop_port = pkt_type_to_next_hop[pkt_type];
                if (dst_port != nxt_hop_port)
                    continue;

                if (find(pkt_types.begin(), pkt_types.end(), pkt_type) == pkt_types.end()) {
                    pkt_types.push_back(pkt_type);
                }
            }
        }
    }
}

int check_wl(vector<string> wl, bool sat) {
    ev3 I;

    map<int, int> pkt_type_to_dst = {
        {0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5},
        {6, 0}, {7, 1}, {8, 2}, {9, 3}, {10, 4}, {11, 5}
    };
    map<int, int> pkt_type_to_ecmp = {
        {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0},
        {6, 1}, {7, 1}, {8, 1}, {9, 1}, {10, 1}, {11, 1}
    };

    SmtSolver slv;


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
    l1 = new DemuxSwitch(slv, "l1", l1_ports, TIME_STEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ,
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
    l2 = new DemuxSwitch(slv, "l2", l2_ports, TIME_STEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ,
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

    l3 = new DemuxSwitch(slv, "l3", l3_ports, TIME_STEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ,
                         l3_pkt_type_to_nxt_hop
    );

    // exit(0);
    LeafBase *s1;
    map<tuple<int, int>, vector<int> > s1_ports = {
        {{0, 1}, {2, 3}},
        {{0, 2}, {4, 5}},
        {{1, 0}, {0, 1}},
        {{1, 2}, {4, 5}},
        {{2, 0}, {0, 1}},
        {{2, 1}, {2, 3}}
    };

    vector s1_pkt_type_to_nxt_hop = {0, 0, 1, 1, 2, 2, 0, 0, 1, 1, 2, 2};
    s1 = new DemuxSwitch(slv, "s1", s1_ports, TIME_STEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ,
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
    s2 = new DemuxSwitch(slv, "s2", s2_ports, TIME_STEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ,
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

    if (sat && debug) {
        slv.s.push();
        // slv.add(NamedExp(query(slv, O), "query").negate());
        // slv.add(NamedExp(query(slv, O), "query"));
        slv.check_sat();
        slv.s.pop();
        cout << "SAT: WL is SAT" << endl;
    }
    if (sat) {
        slv.s.push();
        slv.add(NamedExp(query(slv, O), "query").negate());
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
        slv.s.push();
        slv.add(NamedExp(query(slv, O), "query").negate());
        slv.check_unsat();
        slv.s.pop();
        cout << "UNSAT: WL & !Q is UNSAT" << endl;
    }

    auto end_t = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_t - start_t);
    long result = duration.count();
    return result;
}


int main() {
    string wl_file_path = format("./leaf/fperf/leaf.{}.txt", BUFF_CAP);
    vector<vector<string> > wls = read_wl_file(wl_file_path);
    string out_file_path = format("./leaf/buffy/leaf.{}.txt", BUFF_CAP);
    ofstream out(out_file_path, ios::out);
    out << "scheduler, buf_size, wl_idx, time_millis, solver_res" << endl;
    for (int i = 0; i < wls.size(); ++i) {
        // if (i + 1 < 180)
        // continue;
        // if (i > 50)
        // break;
        auto wl = wls[i];
        string res_stat = wl[0];
        cout << "WL: " << i + 1 << "/" << wls.size() << " " << res_stat << endl;
        if (res_stat == "SKIP") {
            cout << "SKIPPING WORKLOAD!!!!!!!!!!" << i << endl;
            continue;
        }
        bool sat = res_stat == "SAT";
        wl.emplace_back("[1, 10]: cenq(0, t) >= t");
        wl.emplace_back("[1, 10]: dst(0, t) == 5");
        // wl.emplace_back("[1, 10]: cenq(2, t) <= 0");
        // wl.emplace_back("[1, 10]: cenq(3, t) <= 0");
        // wl.emplace_back("[1, 10]: cenq(4, t) <= 0");
        // wl.emplace_back("[1, 10]: cenq(5, t) <= 0");
        int res = check_wl(wl, sat);
        // if (res > 0) {
        // cout << "FAILED:" << i << endl;
        // exit(1);
        // }
        out << "leaf" << "," << 10 << ", " << i << ", " << res << ", " << res_stat << endl;
        // exit(0);
    }
}
