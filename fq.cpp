#include <iostream>

#include "antlr4-runtime.h"
#include"z3++.h"
#include "src/FqChecker.hpp"
#include "src/smt_solver.hpp"
#include "src/utils.hpp"
#include "src/gen/wl_parser.hpp"

using namespace std;
using namespace z3;
using namespace antlr4;

constexpr int MAX_ENQ = 4;
constexpr int MAX_DEQ = 1;

constexpr int NUM_BUFS = 5;
constexpr int TIME_STEPS = 14;
constexpr int PKT_TYPES = 1;
constexpr int BUF_CAP = 10;

vector<NamedExp> query(SmtSolver &slv, const ev3 &out) {
    unsigned int query_thresh = (TIME_STEPS / NUM_BUFS) + 3;
    vector<NamedExp> res;
    expr e = (sum(out[NUM_BUFS - 1], TIME_STEPS - 1) >= slv.ctx.int_val(query_thresh));
    res.emplace_back(e);
    return res;
}

vector<NamedExp> wl(const ev3 &ins) {
    vector<NamedExp> res;
    return res;
}

vector<NamedExp> base_wl(SmtSolver &slv, const ev3 &ins) {
    vector<NamedExp> res;
    cout << ins.size() << endl;
    for (int i = 0; i < NUM_BUFS; ++i) {
        if (i < NUM_BUFS - 1) {
            for (int t = 0; t < TIME_STEPS; ++t) {
                expr e = sum(ins[i], t) >= t + 1;
                res.emplace_back(e);
                // res.emplace_back(sum(ins[i], t) >= t + 1, format("sum(wl[{}][{}]) >= 1", i, t));
            }
        }
    }
    return res;
}

int main(const int argc, const char *argv[]) {
    SmtSolver slv;
    FqChecker sts(slv, "fq", NUM_BUFS, TIME_STEPS, PKT_TYPES, BUF_CAP, MAX_ENQ, MAX_DEQ);
    sts.use_win = false;
    string wl_file_path = "../wls/fq.txt";
    vector<vector<string> > wls = read_wl_file(wl_file_path);
    slv.add(sts.base_constrs());

    auto bwl = base_wl(slv, sts.I);
    slv.add(bwl);

    // slv.add(query(slv, sts.O));

    slv.add(merge(query(slv, sts.O), "not query").negate());
    // slv.add(merge(query(slv, sts.O), "query"));
    for (int i = 0; i < wls.size(); ++i) {
        slv.s.push();
        WorkloadParser parser(sts.I, slv, NUM_BUFS, TIME_STEPS);
        auto wl = wls[i];
        string res_stat = wl[0];
        wl.erase(wl.begin());
        parser.parse(wl);

        if (res_stat == "SAT")
            slv.check_sat();
        else if (res_stat == "UNSAT") {
            try {
                slv.check_unsat();
            } catch (runtime_error e) {
                cout << "ERRRRRRRRRRRRRRRRRRRRRRROR, model is SAT!!!!!!" << endl;
                auto mod = slv.check_sat();
                exit(1);
            }
        }
        cout << "fq, " <<  i << ", " << res_stat << endl;
        slv.s.pop();
    }
    // for (int j = 0; j < NUM_BUFS; ++j) {
    //     if (j == 1) {
    //         continue;
    //     }
    //     for (int i = 0; i < TIME_STEPS; ++i) {
    //         if (i == 0 || i == 1 || i == 2) {
    //             slv.add(sts.B[j][i]);
    //         } else {
    //             slv.add(!sts.B[j][i]);
    //         }
    //     }
    // }

    // auto mod = slv.check_sat();

    // cout << "I:" << endl;
    // cout << str(sts.I, mod).str();
    return 0;
    // cout << "B:" << endl;
    // cout << str(sts.B, mod, "\n").str();
    // cout << "E:" << endl;
    // cout << str(sts.E, mod).str();
    // cout << "OQ:" << endl;
    // cout << str(sts.oq, mod, "\n").str();
    // cout << "NQ:" << endl;
    // cout << str(sts.nq, mod, "\n").str();
    // cout << "Tmp:" << endl;
    // cout << str(sts.tmp, mod, "\n").str() << endl;
    // cout << "IPN:" << endl;
    // cout << str(sts.ipn, mod, "\n").str() << endl;
    // cout << "IPO:" << endl;
    // cout << str(sts.ipo, mod, "\n").str() << endl;
    // cout << "O:" << endl;
    // cout << str(sts.O, mod).str();
    // cout << "B:" << endl;
    // cout << str(sts.B, mod, "\n").str();
    // cout << "I:" << endl;
    // cout << str(sts.I, mod).str();
    // cout << "E:" << endl;
    // cout << str(sts.E, mod).str();
    // cout << "O:" << endl;
    // cout << str(sts.O, mod).str();


    // context *ctx = &slv.ctx;
    // IntSeq is(ctx);
    // expr s = is.create("bar");
    // expr sp = is.push_back(s, 1);
    // slv.add(is.length(s) == 0);
    // slv.add(is.at(s, 0) == 7);
    //
    // auto mod = slv.check_sat();
    // cout << seq_str(sp, mod).str() << endl;
}
