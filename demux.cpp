#include <iostream>
#include<vector>

#include "antlr4-runtime.h"
#include"z3++.h"
#include "src/DemuxSwitch.hpp"
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
constexpr int PKT_TYPES = 3;
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
    // for (const auto &[key, buf]: sts->buffs) {
    //     int src = get<0>(key);
    //     int dst = get<1>(key);
    //     for (int t = 0; t < TIME_STEPS; ++t) {
    //         int val = 0;
    //         if (inp.contains({src, dst, t})) {
    //             val = inp[{src, dst, t}];
    //             cout << "INCLUDES: " << src << " -> " << dst << " @" << t << endl;
    //         }
    //         e = e && (buf->I[t] == val);
    //     }
    // }
    return e;
}

int main(const int argc, const char *argv[]) {
    SmtSolver slv;
    LeafSts *s1;
    vector<tuple<int, int> > s1_ports = {
        {0, 1},
        {0, 2},
    };
    vector<int> pkt_type_to_nxt_hop = {
        -1, 1, 2
    };
    s1 = new DemuxSwitch(slv, "s1", s1_ports, TIME_STEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ,
                         pkt_type_to_nxt_hop
    );

    // in_port, time, type -> count
    map<tuple<int, int, int>, int> ins = {
        {{0, 0, 1}, 2},
        {{0, 0, 2}, 2},
    };
    auto constr = add_constr(s1, ins);
    slv.add({constr, "inp"});

    auto base1 = s1->base_constrs();
    auto base1_merged = merge(base1, "base1");
    slv.add(base1_merged);

    // slv.add(link_ports(s1->get_out_port(2), s2->get_in_port(0)), "link1");
    // slv.add(link_ports(s2->get_out_port(1), s3->get_in_port(2)), "link2");

    auto mod = slv.check_sat();

    cout << "S1" << endl << "##################################" << endl;
    cout << mod.eval(constr) << endl;
    s1->print(mod);
}
