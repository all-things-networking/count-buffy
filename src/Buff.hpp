//
// Created by Amir Hossein Seyhani on 8/24/25.
//

#ifndef BUFF_HPP
#define BUFF_HPP
#include <map>

#include "lib.hpp"
#include "smt_solver.hpp"


class Buff {
public:
    Buff(SmtSolver &slv, const string &var_prefix, int time_steps, int pkt_types, int max_enq, int max_deq,
         int buf_cap, int src, int dst);

    Buff(SmtSolver &slv, const string &var_prefix, int time_steps, int pkt_types, int max_enq, int max_deq, int buf_cap,
         int src, int dst, vector<int> used_pkt_types);

    SmtSolver &slv;
    ev2 I;
    ev2 E;
    ev2 D;
    ev B;
    ev S;
    ev2 O;
    ev2 C;
    ev2 wnd_enq;
    ev2 wnd_enq_nxt;
    ev2 wnd_out;
    ev2 tmp_wnd_enq;
    ev2 tmp_wnd_enq_nxt;
    ev2 tmp_wnd_out;
    ev match;
    int src;
    int dst;
};


#endif //BUFF_HPP
