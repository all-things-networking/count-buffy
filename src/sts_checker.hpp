//
// Created by Amir Hossein Seyhani on 3/17/25.
//

#ifndef STS_CHECKER_HPP
#define STS_CHECKER_HPP
#include "smt_solver.hpp"

class STSChecker {
public:
    virtual ~STSChecker() = default;

    SmtSolver slv;
    evv I;
    evv E;
    evv D;
    evv B;
    evv S;
    evv O;
    evv L;
    int n;
    int k;
    int c;
    int me;
    int md;

    STSChecker(int n, int k, int c, int me, int md);

    model check_wl_sat();

    void check_wl_not_qry_unsat();

    void bl_size(int j);

    void enqs(int j);

    void drops(int j);

    void enq_deq_sum(int j);

    void inputs(int j);

    expr out();

    virtual expr workload() = 0;

    virtual expr out(const ev &bv, const ev &sv, const ev &ov) = 0;

    virtual expr trs(ev const &b, ev const &s, ev const &bp, ev const &sp) = 0;

    virtual expr init(ev const &b0, ev const &s0) = 0;

    void trs();

    virtual expr query(int m) = 0;

    void print(model m);
};


#endif //STS_CHECKER_HPP
