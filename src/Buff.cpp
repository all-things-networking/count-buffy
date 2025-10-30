//
// Created by Amir Hossein Seyhani on 8/24/25.
//

#include "Buff.hpp"

#include "smt_solver.hpp"

const int MAX_I = 10;

Buff::Buff(SmtSolver &slv, const string &var_prefix, int time_steps, int pkt_types, int max_enq, int max_deq,
           int buf_cap, int src, int dst) : slv(slv), src(src), dst(dst) {
    I = slv.ivv(time_steps, pkt_types, format("I_{}", var_prefix));
    E = slv.ivv(time_steps, pkt_types, format("E_{}", var_prefix));
    D = slv.ivv(time_steps, pkt_types, format("D_{}", var_prefix));
    B = slv.bv(time_steps, format("B_{}", var_prefix));
    S = slv.iv(time_steps, format("S_{}", var_prefix));
    O = slv.ivv(time_steps, pkt_types, format("O_{}", var_prefix));
    C = slv.ivv(time_steps, pkt_types, format("C_{}", var_prefix));
    wnd_enq = slv.ivv(time_steps, pkt_types, format("WE_{}", var_prefix));
    wnd_enq_nxt = slv.ivv(time_steps, pkt_types, format("WEN_{}", var_prefix));
    wnd_out = slv.ivv(time_steps, pkt_types, format("WO_{}", var_prefix));
    tmp_wnd_enq = slv.ivv(time_steps, pkt_types, format("TWE_{}", var_prefix));
    tmp_wnd_enq_nxt = slv.ivv(time_steps, pkt_types, format("TWEN_{}", var_prefix));
    tmp_wnd_out = slv.ivv(time_steps, pkt_types, format("TWO_{}", var_prefix));
    match = slv.bv(time_steps, format("Match_{}", var_prefix));
    slv.add_bound(I, 0, MAX_I);
    slv.add_bound(E, 0, max_enq);
    slv.add_bound(D, 0, max_enq);
    slv.add_bound(O, 0, max_deq);
    slv.add_bound(C, 0, buf_cap);
    slv.add_bound(wnd_enq, 0, buf_cap);
    slv.add_bound(wnd_enq_nxt, 0, buf_cap);
    slv.add_bound(wnd_out, 0, buf_cap);
    slv.add_bound(tmp_wnd_enq, 0, buf_cap);
    slv.add_bound(tmp_wnd_enq_nxt, 0, buf_cap);
    slv.add_bound(tmp_wnd_out, 0, buf_cap);
}

Buff::Buff(SmtSolver &slv, const string &var_prefix, int time_steps, int pkt_types, int max_enq, int max_deq,
           int buf_cap, int src, int dst, vector<int> used_pkt_types) : slv(slv), src(src), dst(dst) {
    I = slv.ivv(time_steps, pkt_types, format("I_{}", var_prefix), used_pkt_types);
    E = slv.ivv(time_steps, pkt_types, format("E_{}", var_prefix), used_pkt_types);
    D = slv.ivv(time_steps, pkt_types, format("D_{}", var_prefix), used_pkt_types);
    B = slv.bv(time_steps, format("B_{}", var_prefix));
    S = slv.iv(time_steps, format("S_{}", var_prefix), used_pkt_types);
    O = slv.ivv(time_steps, pkt_types, format("O_{}", var_prefix), used_pkt_types);
    C = slv.ivv(time_steps, pkt_types, format("C_{}", var_prefix), used_pkt_types);
    wnd_enq = slv.ivv(time_steps, pkt_types, format("WE_{}", var_prefix), used_pkt_types);
    wnd_enq_nxt = slv.ivv(time_steps, pkt_types, format("WEN_{}", var_prefix), used_pkt_types);
    wnd_out = slv.ivv(time_steps, pkt_types, format("WO_{}", var_prefix), used_pkt_types);
    tmp_wnd_enq = slv.ivv(time_steps, pkt_types, format("TWE_{}", var_prefix), used_pkt_types);
    tmp_wnd_enq_nxt = slv.ivv(time_steps, pkt_types, format("TWEN_{}", var_prefix), used_pkt_types);
    tmp_wnd_out = slv.ivv(time_steps, pkt_types, format("TWO_{}", var_prefix), used_pkt_types);
    match = slv.bv(time_steps, format("Match_{}", var_prefix));
    slv.add_bound(I, 0, MAX_I);
    slv.add_bound(E, 0, max_enq);
    slv.add_bound(D, 0, max_enq);
    slv.add_bound(O, 0, max_deq);
    slv.add_bound(C, 0, buf_cap);
    slv.add_bound(wnd_enq, 0, buf_cap);
    slv.add_bound(wnd_enq_nxt, 0, buf_cap);
    slv.add_bound(wnd_out, 0, buf_cap);
    slv.add_bound(tmp_wnd_enq, 0, buf_cap);
    slv.add_bound(tmp_wnd_enq_nxt, 0, buf_cap);
    slv.add_bound(tmp_wnd_out, 0, buf_cap);
}
