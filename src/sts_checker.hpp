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
    ev2 S_int;
    ev3 O;
    ev3 C;
    ev3 wnd_enq;
    ev3 wnd_enq_nxt;
    ev3 wnd_out;
    ev3 tmp_wnd_enq;
    ev3 tmp_wnd_enq_nxt;
    ev3 tmp_wnd_out;
    ev2 match;
    int num_bufs;
    int timesteps;
    int pkt_types;
    int c;
    int me;
    int md;
    int k;
    bool use_win;

    STSChecker(SmtSolver &slv, const string &var_prefix, int n, int m, int k, int c, int me, int md);

    vector<NamedExp> base_constrs();

    vector<NamedExp> bl_size(int i) const;

    vector<NamedExp> enqs(int i) const;

    vector<NamedExp> drops(int i);

    vector<NamedExp> enq_deq_sum(int i);

    vector<NamedExp> winds_old(int i);

    void print_stats();

    vector<NamedExp> inputs(int i);

    vector<NamedExp> winds(int i);

    vector<NamedExp> out();

    virtual vector<NamedExp> workload() = 0;

    virtual vector<NamedExp> base_wl();

    virtual vector<NamedExp> out(const ev &bv, const ev &sv, const ev2 &ov, int t) = 0;

    virtual vector<NamedExp> trs(ev const &b, ev const &s, ev const &bp, ev const &sp, int tp) = 0;

    virtual vector<NamedExp> init(ev const &b0, ev const &s0) = 0;

    virtual vector<NamedExp> query() = 0;

    virtual ev2 get_state() const;

    vector<NamedExp> scheduler_constrs();

    vector<NamedExp> input_constrs(int i);

    vector<NamedExp> trs();

    model check_sat(const vector<NamedExp> &v) const;

    void check_unsat(const vector<NamedExp> &v) const;

    model check_wl_and_query_sat();

    void check_wl_and_not_query_unsat();

    virtual void print(model m) const;
};


#endif //STS_CHECKER_HPP
