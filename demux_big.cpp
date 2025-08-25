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
constexpr int PKT_TYPES = 2;
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

int main(const int argc, const char *argv[]) {
    SmtSolver slv;
    LeafSts *s1;
    vector<tuple<int, int> > s1_ports = {
        {0, 2},
        {1, 2},
    };
    vector s1_pkt_type_to_nxt_hop = {2, 2};
    s1 = new DemuxSwitch(slv, "s1", s1_ports, TIME_STEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ,
                         s1_pkt_type_to_nxt_hop
    );

    LeafSts *s2;
    vector<tuple<int, int> > s2_ports = {
        {0, 1}
    };
    vector s2_pkt_type_to_nxt_hop = {1, 1};
    s2 = new DemuxSwitch(slv, "s2", s2_ports, TIME_STEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ,
                         s2_pkt_type_to_nxt_hop
    );

    LeafSts *s3;
    vector<tuple<int, int> > s3_ports = {
        {2, 0},
        {2, 1}
    };
    vector s3_pkt_type_to_nxt_hop = {0, 1};
    s3 = new DemuxSwitch(slv, "s3", s3_ports, TIME_STEPS, PKT_TYPES, BUFF_CAP, MAX_ENQ, MAX_DEQ,
                         s3_pkt_type_to_nxt_hop
    );


    // in_port, time, type -> count
    map<tuple<int, int, int>, int> ins = {
        {{0, 0, 0}, 2},
    };
    auto constr = add_constr(s1, ins);
    slv.add({constr, "inp"});

    auto base1 = s1->base_constrs();
    auto base1_merged = merge(base1, "base1");
    slv.add(base1_merged);

    auto base2 = s2->base_constrs();
    auto base2_merged = merge(base2, "base2");
    slv.add(base2_merged);

    auto base3 = s3->base_constrs();
    auto base3_merged = merge(base3, "base3");
    slv.add(base3_merged);


    slv.add(link_ports(s1->get_out_port(2), s2->get_in_port(0)), "link1");
    slv.add(link_ports(s2->get_out_port(1), s3->get_in_port(2)), "link2");

    auto mod = slv.check_sat();

    cout << "S1" << endl << "##################################" << endl;
    s1->print(mod);

    cout << "S2" << endl << "##################################" << endl;
    s2->print(mod);

    cout << "S3" << endl << "##################################" << endl;
    s3->print(mod);
}
