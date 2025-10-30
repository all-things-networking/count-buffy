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

    // Getter for input events matrix I
    const ev2 getI() const { return I; }
    // Getters for other fields
    const ev2 getE() const { return E; }
    const ev2 getD() const { return D; }
    const ev getB() const { return B; }
    const ev getS() const { return S; }
    const ev2 getO() const { return O; }
    const ev2 getC() const { return C; }
    const ev2 getWndEnq() const { return wnd_enq; }
    const ev2 getWndEnqNxt() const { return wnd_enq_nxt; }
    const ev2 getWndOut() const { return wnd_out; }
    const ev2 getTmpWndEnq() const { return tmp_wnd_enq; }
    const ev2 getTmpWndEnqNxt() const { return tmp_wnd_enq_nxt; }
    const ev2 getTmpWndOut() const { return tmp_wnd_out; }
    const ev getMatch() const { return match; }

    SmtSolver &slv;
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

private:
    ev2 I;
};


#endif //BUFF_HPP
