#include <iostream>
#include<vector>

#include "antlr4-runtime.h"
#include"z3++.h"
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

expr constr(SmtSolver &slv, const ev2 &ev, const vector<int> &v) {
    assert(ev.size() == v.size());
    auto res = slv.ctx.bool_val(true);
    for (int i = 0; i < ev.size(); ++i)
        res = res & (ev[i] == v[i]);
    return res;
}

expr constr(SmtSolver &slv, const ev2 &ev, const vector<vector<int> > &v) {
    assert(ev.size() == v.size());
    auto res = slv.ctx.bool_val(true);
    for (int i = 0; i < ev.size(); ++i)
        res = res & (ev[i] == v[i]);
    return res;
}

void print_mod(STSChecker *sts, model mod) {
    cout << "I:" << endl;
    cout << str(sts->I, mod).str();
    cout << "E:" << endl;
    cout << str(sts->E, mod).str();
    // cout << "B:" << endl;
    // cout << str(sts->B, mod, "\n").str();
    // cout << "S:" << endl;
    // cout << str(sts->S_int, mod, "\n").str();
    cout << "O:" << endl;
    cout << str(sts->O, mod).str();
}

int main(const int argc, const char *argv[]) {
    if (argc < 5)
        return 1;
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    int k = atoi(argv[3]);
    int c = atoi(argv[4]);
    cout << "BUFFER SIZE: " << c << endl;
    string model = "rr";
    SmtSolver slv;
    // PrioSTS *sts;
    // sts = new PrioSTS(slv, model, n, m, k, c, MAX_ENQ, MAX_DEQ);
    RRChecker *sts;
    sts = new RRChecker(slv, model, n, m, k, c, MAX_ENQ, MAX_DEQ);
    string wl_file_path = format("./wls/{}.{}.txt", model, c);
    vector<vector<string> > wls = read_wl_file(wl_file_path);
    string out_file_path = format("./logs/{}.{}.txt", model, c);
    ofstream out(out_file_path, ios::out);
    out << "scheduler, buf_size, wl_idx, time_millis, solver_res" << endl;
    for (int i = 0; i < wls.size(); ++i) {
        cout << "WL: " << i + 1 << "/" << wls.size() << endl;
        WorkloadParser parser(sts->I, slv, n, m);
        auto wl = wls[i];
        slv.s.push();
        slv.add(sts->base_constrs());
        slv.add(sts->base_wl());
        // slv.add(sts->base_wl());
        slv.add(merge(sts->query(4), "Query").negate());
        // slv.add(merge(sts->query(5), "Query"));
        // auto e = sts->B[2][2] && sts->B[2][3] && sts->B[2][4] && sts->B[2][5] && sts->B[2][6];
        // e = e && sts->O[2][2] == 0 && sts->O[2][3] == 0 && sts->O[2][4] == 0 && sts->O[2][5] == 0 && sts->O[2][6] == 0;
        // slv.add(e, "tmp");
        string res_stat = wl[0];
        wl.erase(wl.begin());
        parser.parse(wl);

        auto start_t = chrono::high_resolution_clock::now();
        // auto mod = slv.check_sat();
        if (res_stat == "SAT")
            slv.check_sat();
        else if (res_stat == "UNSAT") {
            try {
                slv.check_unsat();
            } catch (runtime_error e) {
                cout << "ERRRRRRRRRRRRRRRRRRRRRRROR, model is SAT!!!!!!" << endl;
                auto mod = slv.check_sat();
                print_mod(sts, mod);
                exit(1);
            }
        }
        auto end_t = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(end_t - start_t);
        out << "prio, " << c << ", " << i << ", " << duration.count() << ", " << res_stat << endl;
        slv.s.pop();
        continue;
    }
    out.close();

    return 0;
}
