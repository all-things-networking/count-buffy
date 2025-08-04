//
// Created by Amir Hossein Seyhani on 7/28/25.
//

#include "FqChecker.hpp"

#include <support/CPPUtils.h>

FqChecker::FqChecker(SmtSolver &slv, const string &var_prefix, int n, int m, int k, int c, int me, int md)
    : STSChecker(slv, var_prefix, n, m, k, c, me, md) {
    oq = slv.ivv(n, m, "oq");
    nq = slv.ivv(n, m, "nq");
}


vector<NamedExp> FqChecker::out(const ev &bv, const ev &sv, const ev2 &ov, int t) {
    ev nq_t = get_buf_vec_at_i(nq, t);
    ev oq_t = get_buf_vec_at_i(oq, t);

    vector<NamedExp> res;
    expr nq_empty = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i)
        nq_empty = nq_empty && (nq_t[i] < 0);

    for (int i = 0; i < num_bufs; ++i) {
        expr constr = ite((nq_t[i] == 0 || (nq_empty && oq_t[i] == 0)), ov[i] == 1, ov[i] == 0);
        res.emplace_back(constr, format("fq_out_NQ[{}]@{}", i, t));
    }

    return res;
}

vector<NamedExp> FqChecker::trs(const ev &b, const ev &s, const ev &bp, const ev &sp, int tp) {
    vector<NamedExp> res;
    ev nq_t = get_buf_vec_at_i(nq, tp - 1);
    ev oq_t = get_buf_vec_at_i(oq, tp - 1);
    ev nq_tp;
    ev oq_tp;
    ev i_selected_from_nq;

    for (int i = 0; i < num_bufs; ++i) {
        nq_tp.push_back(nq_t[i]);
        oq_tp.push_back(oq_t[i]);
    }


    for (int i = 0; i < num_bufs; ++i) {
        for (int j = 0; j < num_bufs; ++j) {
            // if not backlogged and exists in oq/nq we decrease index
            nq_tp[j] = ite(!bp[i] && nq_t[i] >= 0, nq_tp[j] - 1, nq_tp[j]);
            oq_tp[j] = ite(!bp[i] && oq_tp[i] >= 0, oq_tp[j] - 1, oq_tp[j]);
        }
    }

    expr nq_empty = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i)
        nq_empty = nq_empty && (nq_tp[i] < 0);

    expr pop_from_nq = slv.ctx.bool_val(false);
    //Pop from nq
    for (int i = 0; i < num_bufs; ++i) {
        expr i_selected = !nq_empty && nq_tp[i] == 0;
        i_selected_from_nq.push_back(i_selected);
        pop_from_nq = pop_from_nq || i_selected;
    }

    for (int i = 0; i < num_bufs; ++i) {
        nq_tp[i] = ite(pop_from_nq, nq_tp[i] - 1, nq_tp[i]);
    }

    expr oq_empty = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i)
        oq_empty = oq_empty && (oq_tp[i] < 0);

    expr pop_from_oq = slv.ctx.bool_val(false);

    pop_from_oq = nq_empty && !oq_empty;

    //Pop from oq
    for (int i = 0; i < num_bufs; ++i) {
        oq_tp[i] == ite(pop_from_oq, oq_tp[i] - 1, oq_tp[i]);
    }

    expr max_oq = slv.ctx.int_val(-1);

    for (int i = 0; i < num_bufs; ++i)
        max_oq = max(oq_tp[i], max_oq);

    //Demote to oq
    for (int i = 0; i < num_bufs; ++i) {
        oq_tp[i] = ite(i_selected_from_nq[i] && bp[i], max_oq + 1, oq_tp[i]);
    }

    // Checked till here
    expr max_nq = slv.ctx.int_val(-1);

    for (int i = 0; i < num_bufs; ++i)
        max_nq = max(nq_tp[i], max_nq);

    //Push to nq
    for (int i = 0; i < num_bufs; ++i) {
        expr i_activated = !b[i] && bp[i];
        nq_tp[i] = ite(i_activated, max_nq + 1, nq_tp[i]);
        max_nq = ite(i_activated, max_nq + 1, max_nq);
    }

    for (int i = 0; i < num_bufs; ++i) {
        res.emplace_back(nq[i][tp] == nq_tp[i], format("nq_{}_{}", i, tp));
        res.emplace_back(oq[i][tp] == oq_tp[i], format("oq_{}_{}", i, tp));
    }
    return res;
}

vector<NamedExp> FqChecker::init(const ev &b0, const ev &s0) {
    vector<NamedExp> res;
    for (int i = 0; i < num_bufs; ++i) {
        oq[i][0] = slv.ctx.int_val(-1);
    }
    expr max_idx = slv.ctx.int_val(-1);
    for (int i = 0; i < num_bufs; ++i) {
        max_idx = ite(b0[i], max_idx + 1, max_idx);
        nq[i][0] = ite(b0[i], max_idx, slv.ctx.int_val(-1));
    }

    return res;
}

vector<NamedExp> FqChecker::workload() {
    vector<NamedExp> res;
    return res;
}

expr max(const expr &a, const expr &b) {
    return ite(a >= b, a, b);
}

vector<NamedExp> FqChecker::query(int m) {
    vector<NamedExp> res;
    return res;
}

ev2 FqChecker::get_state() const {
    return SS;
}
