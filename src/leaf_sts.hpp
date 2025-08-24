//
// Created by Amir Hossein Seyhani on 3/17/25.
//

#ifndef PRIO_STS_HPP
#define PRIO_STS_HPP
#include "sts_checker.hpp"

class LeafSts final {
public:
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
    LeafSts(SmtSolver &slv, const string &var_prefix, int n, int m, int k, int c, int me, int md, int num_ports);

    LeafSts(SmtSolver &slv, const string &var_prefix, int n, int num_ports, int c, int me, int md);

    vector<NamedExp> out(const ev &bv, const ev &sv, const ev2 &ov, int t);

    expr rr(ev const &backlog, expr &prev_turn);

    vector<NamedExp> trs(ev const &b, ev const &s, ev const &bp, ev const &sp, int tp);

    vector<NamedExp> init(const ev &b0, const ev &s0);

    vector<NamedExp> base_constrs();

    vector<NamedExp> bl_size(int i) const;

    vector<NamedExp> enqs(int i) const;

    vector<NamedExp> drops(int i);

    vector<NamedExp> enq_deq_sum(int i);

    vector<NamedExp> inputs(int i);

    vector<NamedExp> winds(int i);

    vector<NamedExp> out();

    vector<NamedExp> trs();

    template<class V>
    V get_voq_of_out_i(const V &all_ev, int i);

    void print(model m) const;

    int num_ports;

    ev2 get_state() const;
};


#endif //PRIO_STS_HPP
