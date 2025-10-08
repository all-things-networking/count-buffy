#include <iostream>
#include<vector>
#include"z3++.h"
#include "../src/prio_sts.hpp"
#include "../src/rr_checker.hpp"
#include "../src/trivial_sts.hpp"

using namespace std;
using namespace z3;

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

void do_unsat() {
    STSChecker *sts;
    SmtSolver slv;
    sts = new TrivialSts(slv, "trv", 12);
    auto nes = sts->base_constrs();
    expr e = constr(slv, sts->E[0], {
                        {0, 0},
                        {2, 0},
                        {0, 2},
                        {0, 0},
                        {2, 0},
                        {0, 0},
                        {1, 0},
                        {0, 0},
                        {1, 0},
                        {0, 0},
                        {1, 0},
                        {0, 0}
                    });
    nes.emplace_back(e, "e");

    expr o = constr(slv, sts->O[0], {
                        {0, 0},
                        {0, 0},
                        {0, 0},
                        {2, 0},
                        {0, 0},
                        {2, 0},
                        {0, 0},
                        {1, 0},
                        {0, 0},
                        {1, 0},
                        {1, 2},
                        {0, 0}
                    });
    nes.emplace_back(o, "o");
    sts->check_unsat(nes);
    // auto m = sts->check_sat(nes);
    // sts->print(m);
}

void do_sat() {
    STSChecker *sts;
    SmtSolver slv;
    sts = new TrivialSts(slv, "trv", 12);
    auto nes = sts->base_constrs();
    expr e = constr(slv, sts->E[0], {
                        {0, 0},
                        {2, 0},
                        {0, 2},
                        {0, 0},
                        {2, 0},
                        {0, 0},
                        {1, 0},
                        {0, 0},
                        {1, 0},
                        {0, 0},
                        {1, 0},
                        {0, 0}
                    });
    nes.emplace_back(e, "e");
    expr o = constr(slv, sts->O[0], {
                        {0, 0},
                        {0, 0},
                        {2, 0},
                        {0, 2},
                        {0, 0},
                        {2, 0},
                        {0, 0},
                        {1, 0},
                        {0, 0},
                        {1, 0},
                        {1, 0},
                        {0, 0}
                    });

    nes.emplace_back(o, "o");

    auto mdl = sts->check_sat(nes);
    sts->print(mdl);
}

int main(const int argc, const char *argv[]) {
    do_sat();
    do_unsat();
}
