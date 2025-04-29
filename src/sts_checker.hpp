//
// Created by Amir Hossein Seyhani on 3/17/25.
//

#ifndef STS_CHECKER_HPP
#define STS_CHECKER_HPP
#include "smt_solver.hpp"

class STSChecker {
public:
    virtual ~STSChecker() = default;

    SmtSolver &slv;
    string var_prefix;
    ev3 I;
    ev3 E;
    ev3 D;
    ev2 B;
    ev2 S;
    ev3 O;
    ev3 C;
    ev3 wnd_enq;
    ev3 wnd_enq_nxt;
    ev3 wnd_out;
    int num_bufs;
    int timesteps;
    int pkt_types;
    int c;
    int me;
    int md;

    STSChecker(SmtSolver &slv, string var_prefix, int n, int m, int k, int c, int me, int md);

    model check_wl_sat();

    void check_wl_not_qry_unsat();

    void add_constrs();

    expr base_constrs();

    expr bl_size(int i) const;

    expr enqs(int i) const;

    expr drops(int i);

    expr enq_deq_sum(int i);

    expr inputs(int i);

    void winds(int i);

    expr out();

    virtual expr workload() = 0;

    virtual expr out(const ev &bv, const ev &sv, const ev2 &ov) = 0;

    virtual expr trs(ev const &b, ev const &s, ev const &bp, ev const &sp) = 0;

    virtual expr init(ev const &b0, ev const &s0) = 0;

    expr trs();

    virtual expr query(int m) = 0;

    model check_sat(const expr &e) const;

    void print(model m) const;
};


#endif //STS_CHECKER_HPP
