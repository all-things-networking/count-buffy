//
// Created by Amir Hossein Seyhani on 3/17/25.
//

#include "sts_checker.hpp"
#include "lib.hpp"

STSChecker::STSChecker(const int n, const int k, const int c, const int me, const int md): n(n), k(k), c(c), me(me),
    md(md) {
    I = slv.int_vectors(k, n, "I");
    E = slv.int_vectors(k, n, "E");
    D = slv.int_vectors(k, n, "D");
    B = slv.bool_vectors(k, n, "B");
    S = slv.bool_vectors(k, n, "S");
    O = slv.int_vectors(k, n, "O");
    L = slv.int_vectors(k, n, "L");
    slv.add_bound(I, 0, me);
    slv.add_bound(E, 0, me);
    slv.add_bound(D, 0, me);
    slv.add_bound(O, 0, md);
    slv.add_bound(L, 0, c);
}

model STSChecker::check_wl_sat() {
    slv.s.push();
    slv.add(workload(), "Workload");
    slv.add(query(5), "Query");
    slv.add(out(), "Out");
    trs();
    for (int j = 0; j < k; ++j) {
        inputs(j);
    }
    auto m = slv.check_sat();
    slv.s.pop();
    return m;
}

void STSChecker::check_wl_not_qry_unsat() {
    slv.s.push();
    slv.add(workload(), "Workload");
    slv.add(!query(5), "Query");
    slv.add(out(), "Out");
    trs();
    for (int j = 0; j < k; ++j) {
        inputs(j);
    }
    slv.check_unsat();
    slv.s.pop();
}

void STSChecker::bl_size(int j) {
    const auto Ej = E[j];
    const auto Bj = B[j];
    const auto Lj = L[j];
    const auto Oj = O[j];
    slv.add(Lj[0] == 0, format("S[{}][0] == 0", j));
    for (int i = 1; i < n; ++i) {
        expr e = (implies(!Bj[i], (Lj[i] == slv.ctx.int_val(0))) & (Lj[i] == (Lj[i - 1] + Ej[i] - Oj[i])));
        slv.add(e, format("S[{}][{}] == constr", j, i));
    }
}

void STSChecker::enqs(int j) {
    const auto Ej = E[j];
    const auto Bj = B[j];
    const auto Lj = L[j];
    for (int i = 1; i < n; ++i) {
        expr lt_cap = ((c - Lj[i - 1]) >= Ej[i]);
        expr blogged = (((Ej[i] > 0) || (Lj[i - 1] > 0)) == Bj[i]);
        expr e = blogged && lt_cap;
        slv.add(e, format("Enqs[{}][{}] = constr", j, i));
    }
}


void STSChecker::drops(int j) {
    const auto Dj = D[j];
    const auto Ej = E[j];
    const auto Bj = B[j];
    const auto Lj = L[j];
    for (int i = 1; i < n; ++i) {
        auto e = (ite(Bj[i], implies(Dj[i] > 0, (Lj[i - 1] + Ej[i]) == c), Dj[i] == 0));
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

void STSChecker::trs() {
    slv.add(init(get_buf_vec_at_i(B, 0), get_buf_vec_at_i(S, 0)), "Init");
    for (int i = 0; i < n - 1; ++i) {
        ev const &b = get_buf_vec_at_i(B, i);
        ev const &bp = get_buf_vec_at_i(B, i + 1);
        ev const &s = get_buf_vec_at_i(S, i);
        ev const &sp = get_buf_vec_at_i(S, i + 1);
        auto expr = trs(b, s, bp, sp);
        slv.add(expr, format("Trs({},{})", i, i + 1));
    }
}

void STSChecker::print(model m) {
    cout << "I:" << endl;
    ::print(I, m);
    cout << "E:" << endl;
    ::print(E, m);
    cout << "D:" << endl;
    ::print(D, m);
    cout << "B:" << endl;
    ::print(B, m);
    cout << "S:" << endl;
    ::print(S, m);
    cout << "L:" << endl;
    ::print(S, m);
    cout << "O:" << endl;
    ::print(O, m);
}

expr STSChecker::out() {
    expr res = slv.ctx.bool_val(true);
    for (int i = 0; i < n; ++i) {
        res = res && out(get_buf_vec_at_i(B, i), get_buf_vec_at_i(S, i), get_buf_vec_at_i(O, i));
    }
    return res;
}
