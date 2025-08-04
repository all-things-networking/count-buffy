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

constexpr int MAX_ENQ = 1;
constexpr int MAX_DEQ = 1;

constexpr int NUM_BUFS = 3;
constexpr int TIME_STEPS = 5;
constexpr int PKT_TYPES = 1;
constexpr int BUF_CAP = 10;

int main(const int argc, const char *argv[]) {
    SmtSolver slv;
    FqChecker sts(slv, "fq", NUM_BUFS, TIME_STEPS, PKT_TYPES, BUF_CAP, MAX_ENQ, MAX_DEQ);
    string wl_file_path = "../wls/fq.txt";
    vector<vector<string> > wls = read_wl_file(wl_file_path);

    slv.s.push();
    slv.add(sts.base_constrs());
    WorkloadParser parser(sts.I, slv, NUM_BUFS, TIME_STEPS);
    auto wl = wls[0];
    string res_stat = wl[0];
    wl.erase(wl.begin());
    parser.parse(wl);
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

    auto mod = slv.check_sat();

    cout << "B:" << endl;
    cout << str(sts.B, mod, "\n").str();
    cout << "OQ:" << endl;
    cout << str(sts.oq, mod, "\n").str();
    cout << "NQ:" << endl;
    cout << str(sts.nq, mod, "\n").str();
    cout << "I:" << endl;
    cout << str(sts.I, mod).str();
    cout << "E:" << endl;
    cout << str(sts.E, mod).str();
    cout << "O:" << endl;
    cout << str(sts.O, mod).str();
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
