//
// Created by Amir Hossein Seyhani on 7/28/25.
//

#include "FqChecker.hpp"

#include <support/CPPUtils.h>

FqChecker::FqChecker(SmtSolver &slv, const string &var_prefix, int n, int m, int k, int c, int me, int md)
    : STSChecker(slv, var_prefix, n, m, k, c, me, md) {
    oq = slv.ivv(n, m, "oq");
    nq = slv.ivv(n, m, "nq");
    tmp = slv.bvv(n, m, "tmp");
    ipn = slv.bvv(n, m, "ipn");
    ipo = slv.bvv(n, m, "ipo");

    // slv.add_bound(oq, -10, n);
    // slv.add_bound(nq, -10, n);
}


vector<NamedExp> FqChecker::out(const ev &bv, const ev &sv, const ev2 &ov, int t) {
    ev nq_t = get_buf_vec_at_i(nq, t);
    ev oq_t = get_buf_vec_at_i(oq, t);

    vector<NamedExp> res;
    expr nq_select = slv.ctx.bool_val(false);
    // for (int i = 0; i < num_bufs; ++i)
    // nq_select = nq_select || (ipn[i][t]);

    // for (int i = 0; i < num_bufs; ++i) {
    // expr constr = ite(nq_select,
    // ite(ipn[i][t], ov[i] == 1, ov[i] == 0),
    // ite(ipo[i][t], ov[i] == 1, ov[i] == 0)
    // );

    for (int i = 0; i < num_bufs; ++i)
        nq_select = nq_select || (nq_t[i] == 0);

    for (int i = 0; i < num_bufs; ++i) {
        expr constr = ite(nq_select,
                          ite(nq_t[i] == 0, ov[i] == 1, ov[i] == 0),
                          ite(oq_t[i] == 0, ov[i] == 1, ov[i] == 0)
        );
        // expr constr = ite((nq_t[i] == 0 || (nq_select && oq_t[i] == 0)), ov[i] == 1, ov[i] == 0);
        // expr constr = ite(nq_t[i] == 0, ov[i] == 1)
        // expr constr = ite((nq_t[i] == 0), ov[i] == 1, ov[i] == 0);
        // expr constr = ite(ipn[i][t], ov[i] == 1, ov[i] == 0);
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
    ev i_popped_from_nq;
    ev i_popped_from_oq;

    for (int i = 0; i < num_bufs; ++i) {
        nq_tp.push_back(nq_t[i]);
        oq_tp.push_back(oq_t[i]);
    }

    for (int i = 0; i < num_bufs; ++i) {
        for (int j = 0; j < num_bufs; ++j) {
            nq_tp[j] = ite(!bp[i] && nq_tp[i] >= 0, nq_tp[j] - 1, nq_tp[j]);
            oq_tp[j] = ite(!bp[i] && oq_tp[i] >= 0, oq_tp[j] - 1, oq_tp[j]);
        }
    }

    //Pop from nq
    expr pop_from_nq = slv.ctx.bool_val(false);
    for (int i = 0; i < num_bufs; ++i) {
        expr i_popped = nq_tp[i] == 0;
        i_popped_from_nq.push_back(i_popped);
        pop_from_nq = pop_from_nq || i_popped;
    }
    for (int i = 0; i < num_bufs; ++i) {
        nq_tp[i] = ite(pop_from_nq, nq_tp[i] - 1, nq_tp[i]);
    }

    //Push activated to nq
    expr max_nq = slv.ctx.int_val(-1);
    for (int i = 0; i < num_bufs; ++i)
        max_nq = max(nq_tp[i], max_nq);
    for (int i = 0; i < num_bufs; ++i) {
        expr i_activated = !b[i] && bp[i];
        nq_tp[i] = ite(i_activated, max_nq + 1, nq_tp[i]);
        max_nq = ite(i_activated, max_nq + 1, max_nq);
        tmp[i][tp] = nq_tp[i];
    }

    expr nq_empty_after_enqs = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i) {
        nq_empty_after_enqs = nq_empty_after_enqs && (nq_tp[i] < 0);
    }

    // Pop from OQ
    expr pop_from_oq = slv.ctx.bool_val(false);
    for (int i = 0; i < num_bufs; ++i) {
        expr i_popped = nq_empty_after_enqs && oq_tp[i] == 0;
        i_popped_from_oq.push_back(i_popped);
        pop_from_oq = pop_from_oq || i_popped;
    }
    for (int i = 0; i < num_bufs; ++i) {
        oq_tp[i] = ite(pop_from_oq, oq_tp[i] - 1, oq_tp[i]);
    }


    // Demote to oq
    expr max_oq = slv.ctx.int_val(-1);
    for (int i = 0; i < num_bufs; ++i)
        max_oq = max(oq_tp[i], max_oq);

    for (int i = 0; i < num_bufs; ++i) {
        oq_tp[i] = ite(bp[i] && (i_popped_from_nq[i] || i_popped_from_oq[i]), max_oq + 1, oq_tp[i]);
    }

    for (int i = 0; i < num_bufs; ++i) {
        res.emplace_back(nq[i][tp] == nq_tp[i], format("nq_{}_{}", i, tp));
        res.emplace_back(oq[i][tp] == oq_tp[i], format("oq_{}_{}", i, tp));
        res.emplace_back(ipn[i][tp] == i_popped_from_nq[i], format("isn_{}_{}", i, tp));
        res.emplace_back(ipo[i][tp] == i_popped_from_oq[i], format("iso_{}_{}", i, tp));
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

    for (int i = 0; i < num_bufs; ++i) {
        ipn[i][0] = nq[i][0] == 0;
        ipo[i][0] = slv.ctx.bool_val(false);
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
