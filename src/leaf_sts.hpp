//
// Created by Amir Hossein Seyhani on 3/17/25.
//

#ifndef PRIO_STS_HPP
#define PRIO_STS_HPP
#include <map>

#include "Buff.hpp"
#include "sts_checker.hpp"

class LeafSts final {
public:
    SmtSolver &slv;
    string var_prefix;
    int timesteps;
    int pkt_types;
    int buff_cap;
    int max_enq;
    int max_deq;
    bool use_win;
    ev tmp;

    map<tuple<int, int>, Buff *> buffs;

    map<int, map<int, Buff *> > get_per_dst_buff_map();

    map<int, ev> turn_for_dst;

    LeafSts(SmtSolver &slv,
            const string &var_prefix,
            vector<tuple<int, int> > port_list,
            int time_steps,
            int pkt_types,
            int buf_cap,
            int max_enq,
            int max_deq);

    vector<Buff *> get_buff_list() const;

    vector<NamedExp> out(int t);

    expr rr(const vector<Buff *> &src_buffs, const expr &prev_turn, int t);

    vector<Buff *> get_buffs_for_dst(int dst);

    vector<Buff *> get_buffs_for_src(int src);

    vector<NamedExp> trs(int t);

    vector<NamedExp> init();

    vector<NamedExp> base_constrs();

    vector<NamedExp> bl_size(int i) const;

    vector<NamedExp> enqs(int i) const;

    vector<NamedExp> drops(int i);

    vector<NamedExp> enq_deq_sum(int i);

    vector<NamedExp> inputs(int i);

    vector<NamedExp> winds(int i);

    vector<NamedExp> out();

    vector<NamedExp> trs();

    ev2 get_in_port(int src);

    ev2 get_out_port(int dst);

    template<class V>
    V get_voq_of_out_i(const V &all_ev, int i);

    void print(model m);

    int num_ports;
};


#endif //PRIO_STS_HPP
