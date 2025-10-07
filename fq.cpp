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
            }
        }
    }
    return res;
}

int main(const int argc, const char *argv[]) {
    if (argc < 2)
        return 1;
    int c = atoi(argv[1]);
    SmtSolver slv;
    FqChecker sts(slv, "fq", NUM_BUFS, TIME_STEPS, PKT_TYPES, BUF_CAP, MAX_ENQ, MAX_DEQ);
    sts.use_win = true;
    slv.add(sts.base_constrs());
    string model = "fq";
    string wl_file_path = format("./wls/{}.{}.txt", model, c);
    vector<vector<string> > wls = read_wl_file(wl_file_path);

    string out_file_path = format("./logs/{}.{}.txt", model, c);
    ofstream out(out_file_path, ios::out);
    out << "scheduler, buf_size, wl_idx, time_millis, solver_res" << endl;
    auto bwl = base_wl(slv, sts.I);
    slv.add(bwl);

    slv.add(merge(query(slv, sts.O), "not query").negate());
    for (int i = 0; i < wls.size(); ++i) {
        cout << "WL: " << i + 1 << "/" << wls.size() << endl;
        slv.s.push();
        WorkloadParser parser(sts.I, slv, NUM_BUFS, TIME_STEPS);
        auto wl = wls[i];
        string res_stat = wl[0];
        wl.erase(wl.begin());
        parser.parse(wl);

        auto start_t = chrono::high_resolution_clock::now();
        if (res_stat == "SAT")
            slv.check_sat();
        else if (res_stat == "UNSAT") {
            try {
                slv.check_unsat();
            } catch (runtime_error e) {
                cout << "model is SAT!!!!!!" << endl;
                auto mod = slv.check_sat();
                exit(1);
            }
        }
        auto end_t = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(end_t - start_t);
        out << "fq, " << c << ", " << i << ", " << duration.count() << ", " << res_stat << endl;
        cout << duration.count() << ", " << res_stat << endl;
        slv.s.pop();
    }
    return 0;
}
