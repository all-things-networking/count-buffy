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

constexpr int MAX_ENQ = 10;
constexpr int MAX_DEQ = 1;

constexpr int INP_CALC = 3;

void bar(const vector<int> &v) {
}

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

int main(const int argc, const char *argv[]) {
    if (argc < 5)
        return 1;
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    int k = atoi(argv[3]);
    int c = atoi(argv[4]);
    string name = argv[5];
    STSChecker *sts;
    SmtSolver slv;
    sts = new PrioSTS(slv, "prio", n, m, k, c, MAX_ENQ, MAX_DEQ);

    vector<vector<string> > wls = read_wl_file();
    for (auto wl: wls) {
        WorkloadParser parser(sts->I, slv, n, m);
        slv.s.push();
        slv.add(sts->base_constrs());
        slv.add(merge(sts->query(5), "Query").negate());
        parser.parse(wl);
        auto mod = slv.check_sat();
        slv.s.pop();
        cout << "IT:" << endl;
        cout << str(sts->I, mod).str();
    }

    return 0;
}
