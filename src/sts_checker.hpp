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
    evvv I;
    evvv E;
    evvv D;
    evv B;
    evv S;
    evvv O;
    evvv C;
    int num_bufs;
    int timesteps;
    int pkt_types;
    int c;
    int me;
    int md;

    STSChecker(int n, int m, int k, int c, int me, int md);

    model check_wl_sat();

    void check_wl_not_qry_unsat();

    void bl_size(int i);

    void enqs(int i);

    void drops(int i);

    void enq_deq_sum(int i);

    void inputs(int i);

    expr out();

    virtual expr workload() = 0;

    virtual expr out(const ev &bv, const ev &sv, const evv &ov) = 0;

    virtual expr trs(ev const &b, ev const &s, ev const &bp, ev const &sp) = 0;

    virtual expr init(ev const &b0, ev const &s0) = 0;

    void trs();

    virtual expr query(int m) = 0;

    void print(model m) const;
};


#endif //STS_CHECKER_HPP
