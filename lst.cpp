#include <exprtk.hpp>
#include <iostream>
#include<vector>

#include "antlr4-runtime.h"
#include"z3++.h"
#include "src/leaf_spine/demux_switch.hpp"
#include "src/leaf_spine/leaf_sts.hpp"
#include "src/leaf_spine/leaf_utils.hpp"
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
constexpr int RANDOM_SEED = 6000;

expr query(SmtSolver &slv, ev3 &O) {
    expr s = slv.s.ctx().int_val(0);
    int i = 5;
    for (int t = 0; t < TIME_STEPS; ++t)
        s = s + sum(O[i][t]);
    int thresh = TIME_STEPS / 2;
    cout << "QUERY THRESHOLD:" << thresh << endl;
    expr q = s < thresh;
    return q;
}


map<tuple<int, int>, vector<int> > get_leaf_ports(int hosts_per_leaf, int num_spines) {
    map<tuple<int, int>, vector<int> > leaf_ports;

    int total_ports = hosts_per_leaf + num_spines;

    for (int host_port = 0; host_port < hosts_per_leaf; ++host_port) {
        for (int i = 0; i < total_ports; ++i) {
            if (host_port != i) {
                leaf_ports[{host_port, i}] = {};
                leaf_ports[{i, host_port}] = {};
            }
        }
    }

    return leaf_ports;
}

vector<int> get_port_to_ecmp(int host_per_leaf, int num_spines) {
    vector<int> result;
    for (int i = 0; i < host_per_leaf; ++i) {
        result.push_back(-1);
    }
    for (int i = 0; i < num_spines; ++i) {
        result.push_back(i);
    }
    return result;
}

void print_vec(vector<int> vec) {
    for (int i = 0; i < vec.size(); ++i) {
        cout << vec[i] << ",";
    }
    cout << endl;
}

vector<int> get_pkt_type_to_next_hop_port(int num_spines, int leafs_per_spine, int hosts_per_leaf, int leaf_index) {
    vector<int> result;
    int total_hosts = leafs_per_spine * hosts_per_leaf;
    int pkt_types = num_spines * total_hosts;
    for (int i = 0; i < pkt_types; ++i) {
        int host_index = i % total_hosts;
        bool host_is_in_leaf = (host_index >= leaf_index * hosts_per_leaf) && (
                                   host_index < (leaf_index + 1) * hosts_per_leaf);
        if (host_is_in_leaf) {
            int host_index_in_leaf = host_index % hosts_per_leaf;
            result.push_back(host_index_in_leaf);
        } else {
            int spine_index = i / total_hosts;
            int spine_port = hosts_per_leaf + spine_index;
            result.push_back(spine_port);
        }
    }
    return result;
}

map<int, int> get_pkt_type_to_dst_index(int num_spines, int leafs_per_spine, int hosts_per_leaf) {
    map<int, int> result;
    int total_hosts = leafs_per_spine * hosts_per_leaf;
    int pkt_types = num_spines * total_hosts;
    for (int i = 0; i < pkt_types; ++i) {
        int host_index = i % total_hosts;
        result[i] = host_index;
    }
    return result;
}

map<int, int> get_pkt_type_to_spine_index(int num_spines, int leafs_per_spine, int hosts_per_leaf) {
    map<int, int> result;
    int total_hosts = leafs_per_spine * hosts_per_leaf;
    int pkt_types = num_spines * total_hosts;
    for (int i = 0; i < pkt_types; ++i) {
        int spine_index = i / total_hosts;
        result[i] = spine_index;
    }
    return result;
}

vector<int> get_port_index_to_input_index(int num_spines, int leafs_per_spine, int hosts_per_leaf, int leaf_index) {
    vector<int> result;
    for (int i = 0; i < hosts_per_leaf; ++i) {
        int host_index = hosts_per_leaf * leaf_index + i;
        result.push_back(host_index);
    }
    for (int i = 0; i < num_spines; ++i)
        result.push_back(-1);

    return result;
}

map<tuple<int, int>, vector<int> > get_spine_port_map(int leafs_per_spine, int host_per_leaf, int spine_index) {
    int total_hosts = host_per_leaf * leafs_per_spine;
    map<tuple<int, int>, vector<int> > result;
    for (int i = 0; i < leafs_per_spine; ++i) {
        for (int j = 0; j < leafs_per_spine; ++j) {
            if (i != j) {
                result[{i, j}] = {};
                for (int k = 0; k < host_per_leaf; ++k) {
                    int host_index = host_per_leaf * j + k;
                    int host_pkt_type = spine_index * total_hosts + host_index;
                    result[{i, j}].push_back(host_pkt_type);
                }
            }
        }
    }
    return result;
}

void print_ports(map<tuple<int, int>, vector<int> > ports) {
    for (auto &[src_dst, pkt_types]: ports) {
        cout << get<0>(src_dst) << "->" << get<1>(src_dst) << " : ";
        print_vec(pkt_types);
        cout << endl;
    }
}


vector<int> get_pkt_type_to_next_hop_port_spine(int num_spines, int leafs_per_spine, int host_per_leaf,
                                                int spine_index) {
    vector<int> result;
    int total_hosts = leafs_per_spine * host_per_leaf;
    int pkt_types = num_spines * total_hosts;
    vector s1_pkt_type_to_nxt_hop = {0, 0, 1, 1, 2, 2, 0, 0, 1, 1, 2, 2};
    for (int i = 0; i < pkt_types; ++i) {
        int host_index = i % total_hosts;
        int leaf_index = host_index / host_per_leaf;
        result.push_back(leaf_index);
    }
    return result;
}

int check_wl(vector<string> wl, bool sat, int buff_cap) {
    ev3 I;
    int num_spines = 2;
    int leafs_per_spine = 3;
    int hosts_per_leaf = 2;

    map<int, int> pkt_type_to_dst = get_pkt_type_to_dst_index(num_spines, leafs_per_spine, hosts_per_leaf);
    map<int, int> pkt_type_to_ecmp = get_pkt_type_to_spine_index(num_spines, leafs_per_spine, hosts_per_leaf);

    SmtSolver slv(RANDOM_SEED);

    ev3 tmp_I = slv.ivvv(6, TIME_STEPS, PKT_TYPES, "TMP_I");
    slv.s.push();
    auto vals_map = add_workload(slv, tmp_I, TIME_STEPS, pkt_type_to_dst, pkt_type_to_ecmp, wl);
    slv.s.pop();

    set<int> used_dsts = get_used_vals(vals_map, "dst");
    set<int> used_ecmps = get_used_vals(vals_map, "ecmp");
    set<int> zero_inputs = get_zero_inputs(vals_map);

    vector port_to_ecmp = get_port_to_ecmp(hosts_per_leaf, num_spines);
    print_vec(port_to_ecmp);

    vector<LeafBase *> leafs;

    for (int i = 0; i < leafs_per_spine; ++i) {
        LeafBase *leaf;
        map<tuple<int, int>, vector<int> > leaf_ports = get_leaf_ports(hosts_per_leaf, num_spines);

        vector pkt_type_to_nxt_hop = get_pkt_type_to_next_hop_port(num_spines, leafs_per_spine, hosts_per_leaf, i);
        print_vec(pkt_type_to_nxt_hop);
        vector src_port_to_input = get_port_index_to_input_index(num_spines, leafs_per_spine, hosts_per_leaf, i);
        fix(leaf_ports, pkt_type_to_nxt_hop, used_dsts, used_ecmps, pkt_type_to_dst, pkt_type_to_ecmp, port_to_ecmp,
            src_port_to_input, zero_inputs);
        vector<map<string, set<int> > > vals_per_input;
        for (int j = 0; j < hosts_per_leaf; ++j) {
            int host_index = i * hosts_per_leaf + j;
            cout << "HI: " << host_index << endl;
            vals_per_input.push_back(vals_map[host_index]);
        }
        update_ports(vals_per_input, leaf_ports, src_port_to_input, pkt_type_to_nxt_hop, port_to_ecmp);
        printPorts(leaf_ports);
        string leaf_var_prefix = format("l{}", i);
        cout << leaf_var_prefix << " Ports:" << endl;
        leaf = new DemuxSwitch(slv, leaf_var_prefix, leaf_ports, TIME_STEPS, PKT_TYPES, buff_cap, MAX_ENQ, MAX_DEQ,
                               pkt_type_to_nxt_hop
        );
        leafs.push_back(leaf);
    }

    cout << "Before ports" << endl;
    vector<LeafBase*> spines;

    for (int i = 0; i < num_spines; ++i) {
        LeafBase *spine;
        map<tuple<int, int>, vector<int> > spine_ports =
                get_spine_port_map(leafs_per_spine, hosts_per_leaf, i);
        print_ports(spine_ports);
        vector pkt_type_to_nxt_hop =
                get_pkt_type_to_next_hop_port_spine(num_spines, leafs_per_spine, hosts_per_leaf, i);
        print_vec(pkt_type_to_nxt_hop);
        string spine_var_prefix = format("s{}", i);
        spine = new DemuxSwitch(slv, spine_var_prefix, spine_ports, TIME_STEPS, PKT_TYPES, buff_cap, MAX_ENQ, MAX_DEQ,
                                pkt_type_to_nxt_hop);
        spines.push_back(spine);
    }

    for (auto leaf: leafs) {
        cout << leaf->var_prefix << endl;
        auto base_constrs = leaf->base_constrs();
        auto base_constrs_merged = merge(base_constrs, slv.ctx, leaf->var_prefix);
        slv.add(base_constrs_merged);
    }

    for (auto spine: spines) {
        auto base_spine = spine->base_constrs();
        auto base_spine_merged = merge(base_spine, slv.ctx, spine->var_prefix);
        slv.add(base_spine_merged);
    }

    for (int i = 0; i < leafs.size(); ++i) {
        for (int j = 0; j < spines.size(); ++j) {
            auto leaf = leafs[i];
            auto spine = spines[j];
            int spine_port_in_leaf = hosts_per_leaf + j;
            int leaf_port_in_spine = i;
            slv.add({
                link_ports(leaf->get_out_port(spine_port_in_leaf), spine->get_in_port(leaf_port_in_spine)),
                format("Link: {}_{} -> {}_{}", i, spine_port_in_leaf, j, leaf_port_in_spine)
            });
            slv.add({
                link_ports(spine->get_out_port(leaf_port_in_spine), leaf->get_in_port(spine_port_in_leaf)),
                format("Link: {}_{} -> {}_{}", j, leaf_port_in_spine, i, spine_port_in_leaf)
            });
        }
    }


    ev3 O;
    for (auto l: leafs) {
        for (int i = 0; i < hosts_per_leaf; ++i) {
            I.push_back(l->get_in_port(i));
            O.push_back(l->get_out_port(i));
        }
    }

    map<int, vector<int> > dst_to_pkt_type;
    for (auto &[pkt_type,dst]: pkt_type_to_dst)
        dst_to_pkt_type[dst].push_back(pkt_type);

    map<int, vector<int> > ecmp_to_pkt_type;
    for (auto &[pkt_type,ecmp]: pkt_type_to_ecmp)
        ecmp_to_pkt_type[ecmp].push_back(pkt_type);

    slv.s.push();
    add_workload(slv, I, TIME_STEPS, pkt_type_to_dst, pkt_type_to_ecmp, wl);

    auto start_t = high_resolution_clock::now();

    slv.add(uniq(I, slv, dst_to_pkt_type, I.size(), TIME_STEPS));

    if (sat) {
        slv.s.push();
        slv.add(NamedExp(query(slv, O), "query").negate());
        slv.check_sat();
        slv.s.pop();
        cout << "SAT: WL & !Q is SAT" << endl;
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


int main(const int argc, const char *argv[]) {
    if (argc < 2)
        return 1;
    int buff_cap = atoi(argv[1]);
    string wl_file_path = format("./data/sub_wls/lst/lst.{}.txt", buff_cap);
    vector<vector<string> > wls = read_wl_file(wl_file_path);
    string out_file_path = format("./data/logs/lst/lst.{}.txt", buff_cap);
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
        wl.emplace_back("[1, 10]: cenq(0, t) >= t");
        wl.emplace_back("[1, 10]: dst(0, t) == 5");
        int res = check_wl(wl, sat, buff_cap);
        out << "leaf" << "," << buff_cap << ", " << i << ", " << res << ", " << res_stat << endl;
    }
}
