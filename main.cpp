#include <iostream>
#include<vector>
#include"z3++.h"
#include "src/prio_sts.hpp"
#include "src/rr_checker.hpp"
#include "src/trivial_sts.hpp"

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
    // sts = new TrivialSts(slv, "trv", 1, 12, 2, 4, MAX_ENQ, MAX_DEQ);
    // auto e = sts->base_constrs();
    // e = e & constr(slv, sts->E[0], {
    //                    {0, 0},
    //                    {2, 0},
    //                    {0, 2},
    //                    {0, 0},
    //                    {2, 0},
    //                    {0, 0},
    //                    {1, 0},
    //                    {0, 0},
    //                    {1, 0},
    //                    {0, 0},
    //                    {0, 0},
    //                    {0, 0}
    //                });
    //
    //
    // e = e & constr(slv, sts->D[0], {
    //                    {0, 0},
    //                    {0, 0},
    //                    {0, 0},
    //                    {2, 0},
    //                    {0, 0},
    //                    {2, 0},
    //                    {0, 0},
    //                    {1, 0},
    //                    {0, 0},
    //                    {1, 0},
    //                    {0, 0},
    //                    {0, 0}
    //                });
    // e = e & constr(slv, sts->E[0], {4, 0, 0, 0});

    // auto mdl = sts->check_sat(e);
    // sts->print(mdl);

    // if (name == "prio") {
    //     sts = new PrioSTS(slv, "prio", n, m, k, c, MAX_ENQ, MAX_DEQ);
    //     cout << "PRIO" << endl;
    // } else if (name == "rr") {
    //     sts = new RRChecker(slv, "rr", n, m, k, c, MAX_ENQ, MAX_DEQ);
    //     cout << "RR" << endl;
    // }
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
    // expr o = constr(slv, sts->O[0], {
                        // {0, 0},
                        // {0, 0},
                        // {2, 0},
                        // {0, 2},
                        // {0, 0},
                        // {2, 0},
                        // {0, 0},
                        // {1, 0},
                        // {0, 0},
                        // {1, 0},
                        // {1, 0},
                        // {0, 0}
                    // });

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

    // expr e = constr(slv, sts->E[0], {{2, 2}, {0, 0}, {2, 0}, {0, 0}, {0, 0}, {0, 0}});
    // nes.emplace_back(e, "e");
    // expr o = constr(slv, sts->O[0], {{0, 0}, {2, 0}, {0, 0}, {0, 2}, {2, 0}, {0, 0}});
    // nes.emplace_back(o, "o");
    auto mdl = sts->check_sat(nes);
    // auto mod = sts->check_wl_sat();
    sts->print(mdl);
    // sts->check_wl_not_qry_unsat();
    return 0;
}
