#include <iostream>
#include<vector>

#include "antlr4-runtime.h"
#include"z3++.h"
#include "src/prio_sts.hpp"
#include "src/rr_checker.hpp"
#include "src/gen/constr_extractor.hpp"
#include "src/gen/fperfLexer.h"
#include "src/gen/fperfParser.h"

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
    // auto mod = sts->check_wl_sat();
    // sts->print(mod);
    slv.s.push();

    // string line = "[1, 5]: cenq(2, t) >= t";
    string line = "[1, 7]: SUM_[q in {0, 2, }] cenq(q ,t) == t";
    ANTLRInputStream inputStream(line);

    fperfLexer lexer(&inputStream);
    CommonTokenStream tokens(&lexer);

    fperfParser parser(&tokens);
    auto tree = parser.con();
    ConstrExtractor *visitor = new ConstrExtractor(slv, n, m);
    visitor->visit(tree);
    // slv.add(sts->workload());
    for (int i = 0; i < visitor->constrs.size(); ++i) {
        slv.add(visitor->constrs[i], format("workload_line {}", i));
    }
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            slv.add(sum(sts->I[i][j]) == visitor->IT[i][j], format("I[{}][{}] workload total", i, j));
        }
    }
    slv.add(sts->base_constrs());
    slv.add(merge(sts->query(5), "Query").negate());
    slv.add(merge(visitor->constrs, "Workload constrs"));
    auto mod = slv.check_sat();
    slv.s.pop();
    cout << "IT:" << endl;
    cout << str(visitor->IT, mod, "\n").str();
    // sts->print(mod);
    return 0;
}
