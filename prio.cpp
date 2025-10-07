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
#include "src/sts_runner.hpp"
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
    int num_buffers = atoi(argv[1]);
    int timesteps = atoi(argv[2]);
    int pkt_types = atoi(argv[3]);
    int buf_cap = atoi(argv[4]);
    string model = "prio";
    PrioSTS *sts;
    SmtSolver slv;
    sts = new PrioSTS(slv, model, num_buffers, timesteps, pkt_types, buf_cap, MAX_ENQ, MAX_DEQ);
    sts->use_win = true;
    StsRunner runner(sts, model, buf_cap);
    runner.run(num_buffers, timesteps);
}
