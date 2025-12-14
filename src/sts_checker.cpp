#include "sts_checker.hpp"

#include "lib.hpp"

# define FLUSH false

const int MAX_I = 10;

vector<NamedExp> STSChecker::inputs(const int i) {
    vector<NamedExp> res;
    extend(res, bl_size(i));
    extend(res, enqs(i));
    extend(res, drops(i));
    extend(res, enq_deq_sum(i));
    // extend(res, winds_old(i));
    if (this->use_win)
        extend(res, winds(i));
    return res;
}

vector<NamedExp> STSChecker::base_constrs() {
    vector<NamedExp> res;
    for (int i = 0; i < num_bufs; ++i) {
        auto ie = inputs(i);
        res.insert(res.end(), ie.begin(), ie.end());
    }
    // [4]
    auto trs_constrs = trs();
    res.insert(res.end(), trs_constrs.begin(), trs_constrs.end());
    auto out_constrs = out();
    res.insert(res.end(), out_constrs.begin(), out_constrs.end());
    return res;
}

STSChecker::STSChecker(SmtSolver &slv, const string &var_prefix, const int n, const int m, const int k, const int c,
                       const int me,
                       const int md): slv(slv), var_prefix(move(var_prefix)), num_bufs(n),
                                      timesteps(m), pkt_types(k), c(c), me(me), k(k),
                                      md(md) {
    I = slv.ivvv(n, m, k, format("I_{}", var_prefix));
    E = slv.ivvv(n, m, k, format("E_{}", var_prefix));
    D = slv.ivvv(n, m, k, format("D_{}", var_prefix));
    B = slv.bvv(n, m, format("B_{}", var_prefix));
    S_int = slv.ivv(n, m, format("S_{}", var_prefix));
    O = slv.ivvv(n, m, k, format("O_{}", var_prefix));
    C = slv.ivvv(n, m, k, format("C_{}", var_prefix));
    wnd_enq = slv.ivvv(n, m, k, format("WndEnq_{}", var_prefix));
    wnd_enq_nxt = slv.ivvv(n, m, k, format("WndEnqNxt_{}", var_prefix));
    wnd_out = slv.ivvv(n, m, k, format("WndOut_{}", var_prefix));
    tmp_wnd_enq = slv.ivvv(n, m, k, format("TmpWndEnq_{}", var_prefix));
    tmp_wnd_enq_nxt = slv.ivvv(n, m, k, format("TmpWndEnqNxt_{}", var_prefix));
    tmp_wnd_out = slv.ivvv(n, m, k, format("TmpWndOut_{}", var_prefix));
    match = slv.bvv(n, m, format("Match_{}", var_prefix));
    slv.add_bound(I, 0, MAX_I);
    slv.add_bound(E, 0, me);
    slv.add_bound(D, 0, me);
    slv.add_bound(O, 0, md);
    slv.add_bound(C, 0, c);
    slv.add_bound(wnd_enq, 0, c);
    slv.add_bound(wnd_enq_nxt, 0, c);
    slv.add_bound(wnd_out, 0, c);
    slv.add_bound(tmp_wnd_enq, 0, c);
    slv.add_bound(tmp_wnd_enq_nxt, 0, c);
    slv.add_bound(tmp_wnd_out, 0, c);
    this->use_win = true;
}

vector<NamedExp> STSChecker::bl_size(const int i) const {
    const auto Ei = E[i];
    const auto Bi = B[i];
    const auto Ci = C[i];
    const auto Oi = O[i];
    vector<NamedExp> res;
    // [6]
    auto ne = NamedExp(Ci[0] == Ei[0] - Oi[0]);
    res.push_back(ne);
    for (int j = 1; j < timesteps; ++j) {
        // [7]
        ne = NamedExp(Bi[j] == (Ci[j - 1] + Ei[j] > 0));
        res.push_back(ne);
        // [8]
        ne = NamedExp(Ci[j] == (Ci[j - 1] + Ei[j] - Oi[j]));
        res.push_back(ne);
    }
    if constexpr (FLUSH) {
        ne = NamedExp((!Bi[timesteps - 1]));
        res.push_back(ne);
    }
    return res;
}

std::vector<NamedExp> STSChecker::enqs(const int i) const {
    const auto Ei = E[i];
    const auto Bi = B[i];
    const auto Ci = C[i];

    std::vector<NamedExp> res;

    // [9]
    expr e0 = (Ei[0] <= c) && (Bi[0] == (Ei[0] > 0));
    res.emplace_back(e0);

    for (int j = 1; j < timesteps; ++j) {
        // [10]
        expr lt_cap = ((Ei[j] + Ci[j - 1]) <= c);
        expr ej = lt_cap;
        res.emplace_back(ej);
    }
    return res;
}


std::vector<NamedExp> STSChecker::drops(int i) {
    const auto Di = D[i];
    const auto Ei = E[i];
    const auto Bi = B[i];
    const auto Ci = C[i];

    std::vector<NamedExp> res;

    // [11]
    expr d0 = ite(Bi[0], implies(Di[0] > 0, Ei[0] == c), Di[0] == 0);
    res.emplace_back(d0);

    // timesteps 1..m-1
    for (int j = 1; j < timesteps; ++j) {
        // [12]
        expr dj = ite(Bi[j],
                      implies(Di[j] > 0, (Ci[j - 1] + Ei[j]) == c),
                      Di[j] == 0);
        res.emplace_back(dj);
    }
    return res;
}

std::vector<NamedExp> STSChecker::enq_deq_sum(int i) {
    const auto Ii = I[i];
    const auto Ei = E[i];
    const auto Di = D[i];

    std::vector<NamedExp> res;

    // timestep 0
    res.emplace_back(Ii[0] == (Ei[0] + Di[0]));

    // timesteps 1..m-1
    for (int j = 1; j < timesteps; ++j) {
        expr ij = (Ii[j] == (Ei[j] + Di[j]));
        // [13]
        res.emplace_back(ij);
    }
    return res;
}


vector<NamedExp> STSChecker::winds(int i) {
    vector<NamedExp> nes;
    // [14]
    nes.emplace_back(wnd_enq[i][0] == E[i][0]);
    nes.emplace_back(wnd_out[i][0] == O[i][0]);
    nes.emplace_back(wnd_enq_nxt[i][0] == 0);
    for (int j = 1; j < timesteps; ++j) {
        auto se = tmp_wnd_enq[i][j];
        auto sn = tmp_wnd_enq_nxt[i][j];
        nes.emplace_back(wnd_enq[i][j - 1] + E[i][j] == se + sn);

        auto to = wnd_out[i][j - 1] + O[i][j];
        auto m = se <= to;
        auto constr = ite(m,
                          wnd_enq[i][j] == wnd_enq_nxt[i][j] + sn
                          && wnd_enq_nxt[i][j] == slv.const_vec(pkt_types, 0)
                          && wnd_out[i][j] == to - se
                          ,
                          wnd_enq[i][j] == se
                          && wnd_enq_nxt[i][j] == wnd_enq_nxt[i][j - 1] + sn
                          && wnd_out[i][j] == to
        );
        constr = constr && se <= c && sn <= c - 1 && sn >= 0 && se >= 0 && to >= 0 && to <= 2 * c && wnd_enq_nxt[i][j] <
                 c;
        constr = constr && (wnd_enq[i][j - 1] <= se);
        constr = constr && implies(sum(sn) > 0, sum(se) == c);
        constr = constr && implies(sum(se) < c, sum(sn) == 0);
        constr = constr && !(sum(se) < c && sum(sn) > 0);
        nes.emplace_back(constr);
        nes.emplace_back(wnd_out[i][j] <= wnd_enq[i][j]);
    }
    return nes;
}

ev2 STSChecker::get_state() const {
    return S_int;
}

vector<NamedExp> STSChecker::scheduler_constrs() {
    vector<NamedExp> res;
    auto trs_constrs = trs();
    res.insert(res.end(), trs_constrs.begin(), trs_constrs.end());
    auto out_constrs = out();
    res.insert(res.end(), out_constrs.begin(), out_constrs.end());
    return res;
}

vector<NamedExp> STSChecker::input_constrs(int i) {
}

vector<NamedExp> STSChecker::trs() {
    vector<NamedExp> res;
    ev const &b0 = get_buf_vec_at_i(B, 0);
    ev const &s0 = get_buf_vec_at_i(get_state(), 0);
    auto nes = init(b0, s0);
    extend(res, nes);
    for (int i = 0; i < timesteps - 1; ++i) {
        ev const &b = get_buf_vec_at_i(B, i);
        ev const &bp = get_buf_vec_at_i(B, i + 1);
        ev const &s = get_buf_vec_at_i(get_state(), i);
        ev const &sp = get_buf_vec_at_i(get_state(), i + 1);
        nes = trs(b, s, bp, sp, i + 1);
        extend(res, nes, format("Trs({},{})", i, i + 1));
    }
    return res;
}

model STSChecker::check_sat(const vector<NamedExp> &v) const {
    slv.s.push();
    slv.add(v);
    auto m = slv.check_sat();
    slv.s.pop();
    return m;
}

void STSChecker::check_unsat(const vector<NamedExp> &v) const {
    slv.s.push();
    slv.add(v);
    slv.check_unsat();
    slv.s.pop();
}

model STSChecker::check_wl_and_query_sat() {

    slv.s.push();
    slv.add(this->workload());
    slv.add(this->base_constrs());
    slv.add(this->query());
    auto m = slv.check_sat();
    slv.s.pop();
    return m;
}

void STSChecker::check_wl_and_not_query_unsat() {
    slv.s.push();
    slv.add(this->workload());
    slv.add(this->base_constrs());
    auto query_vector = this->query();
    auto query = merge(query_vector, "query");
    slv.add(query.negate());
    slv.check_unsat();
}

void STSChecker::print(model m) const{
    cout << "E:" << endl;
    cout << str(E, m).str();
    cout << "WE:" << endl;
    cout << str(wnd_enq, m).str();
    cout << "WN:" << endl;
    cout << str(wnd_enq_nxt, m).str();
    cout << "O:" << endl;
    cout << str(O, m).str();
    cout << "WO:" << endl;
    cout << str(wnd_out, m).str();
    // cout << "TE:" << endl;
    // cout << str(tmp_wnd_enq, m).str();
    // cout << "TN:" << endl;
    // cout << str(tmp_wnd_enq_nxt, m).str();
    // cout << "TO:" << endl;
    // cout << str(tmp_wnd_out, m).str();
    cout << "C:" << endl;
    cout << str(C, m).str();
    cout << "D:" << endl;
    cout << str(D, m).str();
    cout << "I:" << endl;
    cout << str(I, m).str();
    cout << "B:" << endl;
    cout << str(B, m, "\n").str();
    cout << "S:" << endl;
    cout << str(get_state(), m, "\n").str();
}

vector<NamedExp> STSChecker::out() {
    vector<NamedExp> res;
    for (int j = 0; j < timesteps; ++j) {
        auto nes = out(get_buf_vec_at_i(B, j), get_buf_vec_at_i(get_state(), j), get_buf_vec_at_i(O, j), j);
        extend(res, nes, format("@{}", j));
    }
    return res;
    // return {merge(res, "out")};
}

vector<NamedExp> STSChecker::base_wl() {
    return {};
}

//
vector<NamedExp> STSChecker::winds_old(int i) {
    vector<NamedExp> nes;
    int cap = c;
    // [14]
    nes.emplace_back(wnd_enq[i][0] == E[i][0], format("WndEnq[{}]@{}", i, 0));
    nes.emplace_back(wnd_out[i][0] == O[i][0], format("WndOut[{}]@{}", i, 0));
    nes.emplace_back(wnd_enq_nxt[i][0] == 0, format("WndNxt[{}]@{}", i, 0));
    for (int j = 1; j < timesteps; ++j) {
        auto te = tmp_wnd_enq[i][j];
        auto tn = tmp_wnd_enq_nxt[i][j];
        // auto to = tmp_wnd_out[i][j];
        // auto m = match[i][j];

        // nes.emplace_back(implies(wnd_enq[i][j - 1] + E[i][j] <= cap, te == wnd_enq[i][j - 1] + E[i][j] && sum(tn) == 0),
        // format("WndEnq[{}]@{}", i, j));
        //
        // nes.emplace_back(implies(wnd_enq[i][j - 1] <= cap && sum(wnd_enq[i][j - 1] + E[i][j]) > cap,
        // te == cap && sum(tn) == sum(
        // wnd_enq[i][j - 1] + E[i][j] + wnd_enq_nxt[i][j - 1]) - cap),
        // format("WndEnqNxt[{}]@{}", i, j));
        // nes.emplace_back(
        // implies(sum(wnd_enq[i][j]) > cap, te == wnd_enq[i][j - 1] && tn == wnd_enq_nxt[i][j - 1] + E[i][j]),
        // format("next_wnd_enq[{}]@{}", i, j));

        // [15]
        auto total_sum = wnd_enq[i][j - 1] + wnd_enq_nxt[i][j - 1] + E[i][j];
        // auto te = ite(total_sum <= c, total_sum, c);


        nes.emplace_back((te + tn) == total_sum, format("Update te + tn[{}]@{}", i, j));
        // [16], [17], [18], [19]
        nes.emplace_back((!(te < cap && tn > 0)) && (te <= cap) && (tn <= cap) && (wnd_enq[i][j - 1] <= te),
                         format("Overflow mechanism[{}]@{}", i, j));
        // [20]
        // nes.emplace_back(to == (wnd_out[i][j - 1] + O[i][j]), format("Update to[{}]@{}", i, j));

        auto to = wnd_out[i][j - 1] + O[i][j];
        // [21]
        // nes.emplace_back(m == (te <= to), format("Match[{}]@{}", i, j));
        auto m = te <= to;
        // [22]
        nes.emplace_back(
            ite(m, wnd_enq[i][j] == tn, ite(total_sum <= cap, wnd_enq[i][j] == total_sum, wnd_enq[i][j] == total_sum)),
            format("Update WE[{}]@{}", i, j));
        // [23]
        nes.emplace_back(ite(m, wnd_enq_nxt[i][j] == 0, wnd_enq_nxt[i][j] == tn), format("Update WN[{}]@{}", i, j));
        // [24]
        nes.emplace_back(ite(m, wnd_out[i][j] == to - te, wnd_out[i][j] == to), format("Update WO[{}]@{}", i, j));
        // [25]
        nes.emplace_back(wnd_out[i][j] <= wnd_enq[i][j], format("WndOut <= WndEnq[{}]@{}", i, j));

        nes.emplace_back(C[i][j] == wnd_enq[i][j] + wnd_enq_nxt[i][j] - wnd_out[i][j], format("equity[{}]@{}", i, j));
    }
    return nes;
}

void STSChecker::print_stats() {
    cout << "Bool vars: " << slv.bool_vars << endl;
    cout << "Int vars: " << slv.int_vars << endl;
}
