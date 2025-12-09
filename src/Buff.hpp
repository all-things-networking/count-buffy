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
    virtual ~Buff() = default;

    Buff(SmtSolver &slv, int src, int dst) : slv(slv), src(src), dst(dst) {
    }

    Buff(SmtSolver &slv, const string &var_prefix, int time_steps, int pkt_types, int max_enq, int max_deq,
         int buf_cap, int src, int dst);

    Buff(SmtSolver &slv, const string &var_prefix, int time_steps, int pkt_types, int max_enq, int max_deq, int buf_cap,
         int src, int dst, vector<int> used_pkt_types);

    Buff(SmtSolver &slv, int time_steps, int pkt_types, int src, int dst);

    virtual ev2 getI() const { return I; }
    virtual ev2 getExpandedI() const;
    virtual ev2 getExpandedO() const;

    ev2 getExpandedC() const;

    virtual ev2 getE() const { return E; }
    virtual ev2 getD() const { return D; }
    virtual ev getB() const { return B; }
    virtual ev getS() const { return S; }
    virtual ev2 getO() const { return O; }
    virtual ev2 getC() const { return C; }
    virtual ev2 getWndEnq() const { return wnd_enq; }
    virtual ev2 getWndEnqNxt() const { return wnd_enq_nxt; }
    virtual ev2 getWndOut() const { return wnd_out; }
    virtual ev2 getTmpWndEnq() const { return tmp_wnd_enq; }
    virtual ev2 getTmpWndEnqNxt() const { return tmp_wnd_enq_nxt; }
    virtual ev2 getTmpWndOut() const { return tmp_wnd_out; }
    virtual ev getMatch() const { return match; }

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
    map<int,int> get_pkt_type_to_local_vec_idx() const;
    bool empty;

private:
    vector<int> used_pkt_types;
    int pkt_types;
};


#endif //BUFF_HPP
