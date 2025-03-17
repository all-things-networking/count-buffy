//
// Created by Amir Hossein Seyhani on 3/17/25.
//

#include "sts_checker.hpp"

STSChecker::STSChecker(const int n, const int k, const int c, const int me, const int md): n(n), k(k), c(c), me(me),
    md(md) {
    I = slv.int_vectors(k, n, "I");
    E = slv.int_vectors(k, n, "E");
    D = slv.int_vectors(k, n, "D");
    B = slv.bool_vectors(k, n, "B");
    O = slv.int_vectors(k, n, "O");
    S = slv.int_vectors(k, n, "S");
    slv.add_bound(I, 0, me);
    slv.add_bound(E, 0, me);
    slv.add_bound(D, 0, me);
    slv.add_bound(O, 0, md);
    slv.add_bound(S, 0, c);
}

void STSChecker::check_wl_sat() {
    slv.s.push();
    slv.add(workload(n), "Workload");
    slv.add(query(5), "Query");
    slv.add(out(), "Out");
    slv.add(trs(B, n), "Trs");
    for (int j = 0; j < k; ++j) {
        inputs(j);
    }
    slv.check_sat();
    slv.s.pop();
}

void STSChecker::check_wl_not_qry_unsat() {
    slv.s.push();
    slv.add(workload(n), "Workload");
    slv.add(!query(5), "Query");
    slv.add(out(), "Out");
    slv.add(trs(B, n), "Trs");
    for (int j = 0; j < k; ++j) {
        inputs(j);
    }
    slv.check_unsat();
    slv.s.pop();
}

void STSChecker::bl_size(int j) {
    const auto Ej = E[j];
    const auto Bj = B[j];
    const auto Sj = S[j];
    const auto Oj = O[j];
    slv.add(Sj[0] == 0, format("S[{}][0] == 0", j));
    for (int i = 1; i < n; ++i) {
        expr e = (implies(!Bj[i], (Sj[i] == slv.ctx.int_val(0))) & (Sj[i] == (Sj[i - 1] + Ej[i] - Oj[i])));
        slv.add(e, format("S[{}][{}] == constr", j, i));
    }
}

void STSChecker::enqs(int j) {
    const auto Ej = E[j];
    const auto Bj = B[j];
    const auto Sj = S[j];
    for (int i = 1; i < n; ++i) {
        expr lt_cap = ((c - Sj[i - 1]) >= Ej[i]);
        expr blogged = (((Ej[i] > 0) || (Sj[i - 1] > 0)) == Bj[i]);
        expr e = blogged && lt_cap;
        slv.add(e, format("Enqs[{}][{}] = constr", j, i));
    }
}


void STSChecker::drops(int j) {
    const auto Dj = D[j];
    const auto Ej = E[j];
    const auto Bj = B[j];
    const auto Sj = S[j];
    for (int i = 1; i < n; ++i) {
        auto e = (ite(Bj[i], implies(Dj[i] > 0, (Sj[i - 1] + Ej[i]) == c), Dj[i] == 0));
        slv.add(e, format("Drops[{}][{}] = constr", j, i));
    }
}

void STSChecker::enq_deq_sum(int j) {
    const auto Ij = I[j];
    const auto Ej = E[j];
    const auto Dj = D[j];
    for (int i = 0; i < n; ++i) {
        auto expr = (Ij[i] == (Ej[i] + Dj[i]));
        slv.add(expr, format("Inp[{}][{}] = constr", j, i));
    }
}

void STSChecker::inputs(const int j) {
    bl_size(j);
    enqs(j);
    drops(j);
    enq_deq_sum(j);
}


expr STSChecker::out() {
    expr res = slv.ctx.bool_val(true);
    for (int i = 0; i < n; ++i) {
        res = res && out(get_buf_vec_at_i(B, i), get_buf_vec_at_i(O, i));
    }
    return res;
}

