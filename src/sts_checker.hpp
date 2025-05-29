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
    // ev3 tmp_wnd_enq;
    // ev3 tmp_wnd_enq_nxt;
    // ev3 tmp_wnd_out;
    // ev2 match;
    int num_bufs;
    int timesteps;
    int pkt_types;
    int c;
    int me;
    int md;

    STSChecker(SmtSolver &slv, const string &var_prefix, int n, int m, int k, int c, int me, int md);

    model check_wl_sat();

    void check_wl_not_qry_unsat();

    void add_constrs();

    vector<NamedExp> base_constrs();

    vector<NamedExp> bl_size(int i) const;

    vector<NamedExp> enqs(int i) const;

    vector<NamedExp> drops(int i);

    vector<NamedExp> enq_deq_sum(int i);

    vector<NamedExp> inputs(int i);

    vector<NamedExp> winds(int i);

    vector<NamedExp> out();

    virtual vector<NamedExp> workload() = 0;

    virtual vector<NamedExp> out(const ev &bv, const ev &sv, const ev2 &ov) = 0;

    virtual vector<NamedExp> trs(ev const &b, ev const &s, ev const &bp, ev const &sp) = 0;

    virtual vector<NamedExp> init(ev const &b0, ev const &s0) = 0;

    virtual vector<NamedExp> query(int m) = 0;

    vector<NamedExp> to_uniqe(vector<NamedExp> &v) const;

    vector<NamedExp> scheduler_constrs();

    vector<NamedExp> input_constrs(int i);

    vector<NamedExp> trs();

    model check_sat(const vector<NamedExp> &v) const;

    void check_unsat(const vector<NamedExp> &v) const;

    void print(model m) const;
};


#endif //STS_CHECKER_HPP
