//
// Created by Amir Hossein Seyhani on 3/17/25.
//

#ifndef PRIO_STS_HPP
#define PRIO_STS_HPP
#include "Buff.hpp"
#include "sts_checker.hpp"

class LeafSts final {
public:
    SmtSolver &slv;
    string var_prefix;
    int num_bufs;
    int timesteps;
    int pkt_types;
    int buff_cap;
    int max_enq;
    int max_deq;
    bool use_win;

    vector<Buff> buffs;

    LeafSts(SmtSolver &slv, const string &var_prefix, int num_bufs, int time_steps, int pkt_types, int buf_cap,
            int max_enq, int max_deq, int num_ports);

    LeafSts(SmtSolver &slv, const string &var_prefix, int num_bufs, int num_ports, int buff_cap, int max_enq,
            int max_deq);

    vector<Buff> get_buff_list() const;

    vector<NamedExp> out(int t);

    expr rr(ev const &backlog, expr &prev_turn);

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

    template<class V>
    V get_voq_of_out_i(const V &all_ev, int i);

    void print(model m) const;

    int num_ports;
};


#endif //PRIO_STS_HPP
