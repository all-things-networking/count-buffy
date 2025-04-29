#include "sts_checker.hpp"

#include "lib.hpp"

void STSChecker::add_constrs() {
    slv.add(out(), "Out");
    slv.add(trs(), "Trs");
    for (int i = 0; i < num_bufs; ++i) {
        inputs(i);
    }
}

expr STSChecker::base_constrs() {
    auto res = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i) {
        res = res && inputs(i);
    }
    return res;
}

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
    slv.add(trs(), "Trs");
    for (int i = 0; i < num_bufs; ++i) {
        slv.add(inputs(i), format("Inputs[{}]", i));
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
    slv.add(trs(), "Trs");
    for (int i = 0; i < num_bufs; ++i) {
        slv.add(inputs(i), format("Inputs[{}]", i));
    }
    slv.check_unsat();
    slv.s.pop();
}

expr STSChecker::bl_size(const int i) const {
    const auto Ei = E[i];
    const auto Bi = B[i];
    const auto Ci = C[i];
    const auto Oi = O[i];
    auto res = Ci[0] == 0;
    for (int j = 1; j < timesteps; ++j) {
        expr e = (implies(!Bi[j], (Ci[j] == 0)) & (Ci[j] == (Ci[j - 1] + Ei[j] - Oi[j])));
        res = res & e;
    }
    return res;
}

expr STSChecker::enqs(const int i) const {
    const auto Ei = E[i];
    const auto Bi = B[i];
    const auto Ci = C[i];
    auto res = (Ei[0] <= c) & (Bi[0] == (Ei[0] > 0));
    for (int j = 1; j < timesteps; ++j) {
        expr lt_cap = ((Ei[j] + Ci[j - 1]) <= c);
        expr blogged = (((Ei[j] + Ci[j - 1]) > 0) == Bi[j]);
        expr e = blogged && lt_cap;
        res = res & e;
    }
    return res;
}


expr STSChecker::drops(int i) {
    const auto Di = D[i];
    const auto Ei = E[i];
    const auto Bi = B[i];
    const auto Ci = C[i];
    auto res = ite(Bi[0], implies(Di[0] > 0, (Ei[0]) == c), Di[0] == 0);
    for (int j = 1; j < timesteps; ++j) {
        auto e = (ite(Bi[j], implies(Di[j] > 0, (Ci[j - 1] + Ei[j]) == c), Di[j] == 0));
        res = res & e;
    }
    return res;
}

expr STSChecker::enq_deq_sum(int i) {
    const auto Ii = I[i];
    const auto Ei = E[i];
    const auto Di = D[i];
    auto res = (Ii[0] == (Ei[0] + Di[0]));
    for (int j = 1; j < timesteps; ++j) {
        auto expr = (Ii[j] == (Ei[j] + Di[j]));
        slv.add(expr, format("Inp[{}][{}] = constr", i, j));
    }
    return res;
}

expr STSChecker::inputs(const int i) {
    auto res = bl_size(i);
    res = res & enqs(i);
    res = res & drops(i);
    res = res & enq_deq_sum(i);
    return res;
}

void STSChecker::winds(int i) {
    auto res = wnd_enq[i][0] == E[i][0];
    res = res & wnd_out[i][0] == 0;
    res = res & wnd_enq_nxt[i][0] == 0;
    for (int j = 1; j < timesteps; ++j) {
        res = res & (wnd_out[i][j] <= wnd_enq[i][j]);
        res = res & (wnd_out[i][j] == wnd_out[i][j - 1] + O[i][j]);
        auto overflow = (wnd_enq[i][j - 1] + E[i][j]) > c;
        res = res & (implies(~overflow, wnd_enq[i][j] == wnd_enq[i][j - 1] + E[i][j]));
        auto pred = (wnd_enq[i][j - 1] + wnd_enq_nxt[i][j] == wnd_enq[i][j - 1] + E[i][j])
                    & (wnd_enq[i][j - 1] <= wnd_enq[i][j])
                    & (wnd_enq[i][j] == c);
        res = res & (implies(overflow & wnd_enq_nxt[i][j] == 0, pred));
        res = res & implies(wnd_enq_nxt[i][j - 1] > 0,
                            wnd_enq_nxt[i][j] == wnd_enq_nxt[i][j - 1] + E[i][j] & wnd_enq[i][j] == wnd_enq[i][j - 1]);
        res = res & implies(wnd_out[i][j - 1] == wnd_enq[i][j - 1],
                            wnd_out[i][j] == 0 & wnd_enq[i][j] == wnd_enq_nxt[i][j] & wnd_enq_nxt[i][j] == 0);
    }
    slv.add(res, format("Winds[{}][0] == constr", i));
}

expr STSChecker::trs() {
    get_buf_vec_at_i(B, 0);
    ev const &b0 = get_buf_vec_at_i(B, 0);
    ev const &s0 = get_buf_vec_at_i(S, 0);
    auto res = init(b0, s0);
    for (int i = 0; i < timesteps - 1; ++i) {
        ev const &b = get_buf_vec_at_i(B, i);
        ev const &bp = get_buf_vec_at_i(B, i + 1);
        ev const &s = get_buf_vec_at_i(S, i);
        ev const &sp = get_buf_vec_at_i(S, i + 1);
        res = res & trs(b, s, bp, sp);
    }
    return res;
}

model STSChecker::check_sat(const expr &e) const {
    slv.s.push();
    slv.add(e, "Constraints");
    auto m = slv.check_sat();
    slv.s.pop();
    return m;
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
