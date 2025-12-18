//
// Created by Amir Hossein Seyhani on 10/28/25.
//

#ifndef LEAF_BASE_HPP
#define LEAF_BASE_HPP

#include <map>

#include "LeafBuff.hpp"
#include "../sts_checker.hpp"

class SmtSolver;

class LeafBase {
public:
    virtual ~LeafBase() = default;

    SmtSolver &slv;
    string var_prefix;
    int timesteps;
    int pkt_types;
    int buff_cap;
    int max_enq;
    int max_deq;
    bool use_win;
    map<int, ev> tmp_per_src;
    map<int, ev> tmp_per_dst;

    map<tuple<int, int>, LeafBuff *> buffs;

    map<int, map<int, LeafBuff *> > src_map_per_dst();

    map<int, map<int, LeafBuff *> > dst_map_per_src();

    map<int, ev> src_turn_for_dst;

    map<int, ev> dst_turn_for_src;

    map<tuple<int, int>, ev> matched;

    LeafBase(SmtSolver &slv,
             const string &var_prefix,
             vector<tuple<int, int> > port_list,
             int time_steps,
             int pkt_types,
             int buf_cap,
             int max_enq,
             int max_deq);

    LeafBase(SmtSolver &slv,
             const string &var_prefix,
             map<tuple<int, int>, vector<int> > port_list,
             int time_steps,
             int pkt_types,
             int buf_cap,
             int max_enq,
             int max_deq);

    vector<LeafBuff *> get_buff_list() const;

    virtual vector<NamedExp> out(int t) = 0;

    virtual vector<NamedExp> trs(int t) = 0;

    virtual vector<NamedExp> init() = 0;


    expr rr_for_dst(const vector<LeafBuff *> &buffs, int t, expr prev_turn);

    expr rr_for_src(const vector<LeafBuff *> &buffs, int t, expr prev_turn);

    vector<LeafBuff *> get_buffs_for_dst(int dst);

    vector<LeafBuff *> get_buffs_for_src(int src);

    virtual ev2 get_in_port(int src);

    vector<int> get_in_ports();

    vector<int> get_out_ports();

    ev2 get_out_port(int dst);

    void print(model m);

    int num_ports;

    vector<NamedExp> base_constrs();

    vector<NamedExp> bl_size(int i) const;

    vector<NamedExp> enqs(int i) const;

    vector<NamedExp> drops(int i);

    vector<NamedExp> enq_deq_sum(int i);

    vector<NamedExp> inputs(int i);

    vector<NamedExp> winds(int i);

    vector<NamedExp> out();

    vector<NamedExp> winds_old(int i);

    vector<NamedExp> trs();
};


#endif //LEAF_BASE_HPP
