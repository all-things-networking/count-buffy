#include "sts_checker.hpp"

#include <utility>
#include "lib.hpp"

STSChecker::STSChecker(SmtSolver &slv, string var_prefix, const int n, const int m, const int k, const int c,
                       const int me,
                       const int md): slv(slv), var_prefix(move(var_prefix)), num_bufs(n),
                                      timesteps(m), pkt_types(k), c(c), me(me),
                                      md(md) {
    I = slv.ivvv(n, m, k, format("I_{}", var_prefix));
    E = slv.ivvv(n, m, k, format("E_{}", var_prefix));
    D = slv.ivvv(n, m, k, format("D_{}", var_prefix));
    B = slv.bvv(n, m, format("B_{}", var_prefix));
    S = slv.bvv(n, m, format("S_{}", var_prefix));
    O = slv.ivvv(n, m, k, format("O_{}", var_prefix));
    C = slv.ivvv(n, m, k, format("C_{}", var_prefix));
    slv.add_bound(I, 0, me);
    slv.add_bound(E, 0, me);
    slv.add_bound(D, 0, me);
    slv.add_bound(O, 0, md);
    slv.add_bound(C, 0, c);
}

model STSChecker::check_wl_sat() {
    slv.s.push();
    slv.add(workload(), "Workload");
    slv.add(query(5), "Query");
    slv.add(out(), "Out");
    trs();
    for (int i = 0; i < num_bufs; ++i) {
        inputs(i);
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
    for (int i = 0; i < num_bufs; ++i) {
        inputs(i);
    }
    slv.check_unsat();
    slv.s.pop();
}

void STSChecker::bl_size(int i) {
    const auto Ei = E[i];
    const auto Bi = B[i];
    const auto Ci = C[i];
    const auto Oi = O[i];
    slv.add(Ci[0] == 0, format("S[{}][0] == 0", i));
    for (int j = 1; j < timesteps; ++j) {
        expr e = (implies(!Bi[j], (Ci[j] == 0)) & (Ci[j] == (Ci[j - 1] + Ei[j] - Oi[j])));
        slv.add(e, format("S[{}][{}] == constr", i, j));
    }
}

void STSChecker::enqs(int i) {
    const auto Ei = E[i];
    const auto Bi = B[i];
    const auto Ci = C[i];
    for (int j = 1; j < timesteps; ++j) {
        expr lt_cap = ((Ei[j] + Ci[j - 1]) <= c);
        expr blogged = (((Ei[j] + Ci[j - 1]) > 0) == Bi[j]);
        expr e = blogged && lt_cap;
        slv.add(e, format("Enqs[{}][{}] = constr", i, j));
    }
}


void STSChecker::drops(int i) {
    const auto Di = D[i];
    const auto Ei = E[i];
    const auto Bi = B[i];
    const auto Ci = C[i];
    for (int j = 1; j < timesteps; ++j) {
        auto e = (ite(Bi[j], implies(Di[j] > 0, (Ci[j - 1] + Ei[j]) == c), Di[j] == 0));
        slv.add(e, format("Drops[{}][{}] = constr", i, j));
    }
}

void STSChecker::enq_deq_sum(int i) {
    const auto Ii = I[i];
    const auto Ei = E[i];
    const auto Di = D[i];
    for (int j = 0; j < timesteps; ++j) {
        auto expr = (Ii[j] == (Ei[j] + Di[j]));
        slv.add(expr, format("Inp[{}][{}] = constr", i, j));
    }
}

void STSChecker::inputs(const int i) {
    bl_size(i);
    enqs(i);
    drops(i);
    enq_deq_sum(i);
}

void STSChecker::trs() {
    get_buf_vec_at_i(B, 0);
    ev const &b0 = get_buf_vec_at_i(B, 0);
    ev const &s0 = get_buf_vec_at_i(S, 0);
    slv.add(init(b0, s0), "Init");
    for (int i = 0; i < timesteps - 1; ++i) {
        ev const &b = get_buf_vec_at_i(B, i);
        ev const &bp = get_buf_vec_at_i(B, i + 1);
        ev const &s = get_buf_vec_at_i(S, i);
        ev const &sp = get_buf_vec_at_i(S, i + 1);
        auto expr = trs(b, s, bp, sp);
        slv.add(expr, format("Trs({},{})", i, i + 1));
    }
}

void STSChecker::print(model m) const {
    cout << "I:" << endl;
    cout << str(I, m).str();
    cout << "E:" << endl;
    cout << str(E, m).str();
    cout << "D:" << endl;
    cout << str(D, m).str();
    cout << "B:" << endl;
    cout << str(B, m, "\n").str();
    cout << "S:" << endl;
    cout << str(S, m, "\n").str();
    cout << "C:" << endl;
    cout << str(C, m).str();
    cout << "O:" << endl;
    cout << str(O, m).str();
}

expr STSChecker::out() {
    expr res = slv.ctx.bool_val(true);
    for (int j = 0; j < timesteps; ++j) {
        res = res && out(get_buf_vec_at_i(B, j), get_buf_vec_at_i(S, j), get_buf_vec_at_i(O, j));
    }
    return res;
}
