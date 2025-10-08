#include <iostream>
#include <set>

#include"z3++.h"
#include "../src/rr_checker.hpp"
#include "../src/merger.hpp"

using namespace std;
using namespace z3;

constexpr int MAX_ENQ = 4;
constexpr int MAX_DEQ = 1;

const int TIME_STEPS = 10;
const int RR_IN_BUFS = 2;
const int PKT_TYPES = 2;
const int C = 1000;

vector<NamedExp> query(SmtSolver &slv, ev2 &out) {
    vector<NamedExp> res;
    ev s = out[0];
    for (int i = 1; i < out.size(); ++i)
        s = s + out[i];
    return {{(s[0] - s[1]) >= 3, "Query"}};
}

vector<NamedExp> wl(const ev4 &ins) {
    vector<NamedExp> res;
    for (int l = 0; l < ins.size(); ++l) {
        for (int i = 0; i < RR_IN_BUFS; ++i) {
            for (int t = 0; t < ins[0][i].size(); ++t) {
                if (l == 0) {
                    res.emplace_back(ins[l][i][t] >= 1, format("sum(wl[{}][{}][{}]) >= 1", l, i, t));
                } else {
                    if (i == 0) {
                        res.emplace_back(ins[l][i][t] == 1, format("sum(wl[{}][{}][{}]) == 0", l, i, t));
                    } else {
                        res.emplace_back(ins[l][i][t] >= 1, format("sum(wl[{}][{}][{}]) >= 1", l, i, t));
                    }
                }
            }
        }
    }
    return res;
}

vector<NamedExp> base_wl(const ev4 &ins) {
    vector<NamedExp> res;
    const int num_rrs = ins.size();
    const int total_time = ins[0][0].size();
    for (int l = 0; l < num_rrs; ++l) {
        for (int t = 0; t < total_time; ++t) {
            res.emplace_back(ins[l][0][t][0] > 0, format("I[{}][{}][{}][{}] > 0", l, 0, t, 0));
            res.emplace_back(ins[l][0][t][1] == 0, format("I[{}][{}][{}][{}] == 0", l, 0, t, 1));
            res.emplace_back(ins[l][1][t][0] == 0, format("I[{}][{}][{}][{}] == 0", l, 1, t, 0));
            // res.emplace_back(ins[l][1][t][1] > 0, format("I[{}][{}][{}][{}] > 0", l, 1, t, 1));
            if (l == 0)
                res.emplace_back(ins[l][1][t][1] > 0, format("I[{}][{}][{}][{}] > 0", l, 1, t, 1));
            else
                res.emplace_back(ins[l][1][t][1] == 0, format("I[{}][{}][{}][{}] == 0", l, 1, t, 1));
        }
    }

    // const int num_rrs = ins.size();
    // const int total_time = ins[0][0].size();
    // for (int i = 0; i < RR_IN_BUFS; ++i) {
    //     for (int t = 0; t < total_time; ++t) {
    //         auto s = ins[0][i][t];
    //         for (int l = 1; l < num_rrs; ++l)
    //             s = s + ins[l][i][t];
    //         for (int k = 0; k < PKT_TYPES; ++k) {
    //             if (i == k)
    //                 res.emplace_back(s[k] >= 1, format("sum(base_wl[*][{}][{}][{}]) >= 1", i, t, k));
    //             else
    //                 res.emplace_back(s[k] == 0, format("sum(base_wl[*][{}][{}][{}]) == 0", i, t, k));
    //         }
    //     }
    // }
    return res;
}

class Composed {
    STSChecker *rr1;
    STSChecker *rr2;
    STSChecker *rr3;
    STSChecker *rr4;
    STSChecker *merger;
    SmtSolver slv;
    ev2 O;

public:
    Composed() {
        rr1 = new RRChecker(slv, "rr1", RR_IN_BUFS, TIME_STEPS, PKT_TYPES, C, MAX_ENQ, MAX_DEQ);
        rr2 = new RRChecker(slv, "rr2", RR_IN_BUFS, TIME_STEPS, PKT_TYPES, C, MAX_ENQ, MAX_DEQ);
        rr3 = new RRChecker(slv, "rr3", RR_IN_BUFS, TIME_STEPS, PKT_TYPES, C, MAX_ENQ, MAX_DEQ);
        rr4 = new RRChecker(slv, "rr4", RR_IN_BUFS, TIME_STEPS, PKT_TYPES, C, MAX_ENQ, MAX_DEQ);
        merger = new Merger(slv, "mg", 4, TIME_STEPS, PKT_TYPES, C, MAX_ENQ, MAX_DEQ);
        O = merger->O[0] + merger->O[1] + merger->O[2] + merger->O[3];
        slv.add_bound({O}, 0, MAX_ENQ);
    }

    model run() {
        slv.s.push();

        // vector<STSChecker *> rrs = {rr1, rr2, rr3, rr4};
        // for (int i = 0; i < rrs.size(); ++i) {
            // auto constrs = rrs[i]->base_constrs();
            // auto constrs = rrs[i]->scheduler_constrs();
            // slv.add(constrs);
            // slv.add({
                // rrs[i]->O[0] + rrs[i]->O[1] == merger->I[i], format("mg.I[{}] == rr{}.O[0] + rr{}.O[1])", i, i, i)
            // });
        // }
        // slv.add(merger->base_constrs());
        // slv.add({merger->I[0][0] > 0, "bar"});
        // slv.add({rr1->I[0][0][0] > 1, "baz"});
        // auto ins = {rr1->I, rr2->I, rr3->I, rr4->I};

        // auto base_wl_constrs = base_wl(ins);
        // slv.add(base_wl_constrs);
        // slv.add({rr1->E[0][0] == 2, "x"});
        // slv.add({rr2->E[0][0] == 3, "y"});
        // slv.add(query(slv, O));
        // slv.add(merge(query(slv, O), "not query").negate());
        // auto m = slv.check_unsat();
        // slv.check_unsat();
        // slv.add(wl(ins));
        auto m = slv.check_sat();
        slv.s.pop();
        return m;
    }

    void print(model m) {
        for (auto rr: {rr1, rr2, rr3, rr4}) {
            cout << rr->var_prefix << endl;
            cout << "E:" << endl << str(rr->E, m).str();
            cout << "O:" << endl << str(rr->O, m).str();
            cout << "---------------" << endl;
        }
        cout << merger->var_prefix << endl;
        cout << "E:" << endl << str(merger->E, m).str();
        cout << "O:" << endl << str(O, m, ",").str();
        cout << "---------------" << endl;
    }
};

// Compare time with fperf
// stats: num variables
// Rename max_enq to max_inp
// Add slides for classifier stuff
// Separate type streams and buffers

int main(const int argc, const char *argv[]) {
    Composed c;
    SmtSolver slv;
    // auto x = slv.ctx.bool_const("x");
    // auto y = slv.ctx.bool_const("y");
    // slv.add({x && y, "x && y"});
    // slv.add({x || y, "x || y"});
    // slv.check_sat();
    // cout << slv.stats_str() << endl;
    auto m = c.run();
    // c.print(m);
}
