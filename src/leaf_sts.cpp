#include "leaf_sts.hpp"

#include "prio_sts.hpp"

const int MAX_I = 10;

LeafSts::LeafSts(SmtSolver &slv, const string &var_prefix, const int n, const int m, const int k, const int c,
                 const int me,
                 const int md,
                 const int num_ports): slv(slv), var_prefix(move(var_prefix)), num_bufs(n),
                                       timesteps(m), pkt_types(k), c(c), me(me), k(k),
                                       md(md),num_ports(num_ports) {
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

LeafSts::LeafSts(SmtSolver &slv, const string &var_prefix, int num_ports, int time_steps, int c, int me,
                 int md): LeafSts(slv, var_prefix, num_ports * num_ports, time_steps, 1, c, me, md, num_ports) {
}

vector<NamedExp> LeafSts::out(const ev &bv, const ev &sv, const ev2 &ov, int t) {
    expr res = slv.ctx.bool_val(true);
    for (int src = 0; src < num_ports; ++src) {
        for (int dst = 0; dst < num_ports; ++dst) {
            int idx = src * num_ports + dst;
            expr turn = sv[dst];
            res = res && ite(bv[idx] && turn == slv.ctx.int_val(src), ov[idx] == 1, ov[idx] == 0);
        }
    }
    return {res};
}

expr LeafSts::rr(ev const &backlog, expr &prev_turn) {
    int count = backlog.size();
    expr nxt_turn = slv.ctx.int_val(0);
    for (int i = 0; i < count; ++i) {
        expr x = slv.ctx.int_val(i);
        for (int j = 1; j < count; ++j) {
            const int l = (i - j + count) % count;
            x = ite(backlog[l], slv.ctx.int_val(l), x);
        }
        nxt_turn = ite(prev_turn == i, x, nxt_turn);
    }
    return nxt_turn;
}

vector<NamedExp> LeafSts::trs(ev const &b, ev const &s, ev const &bp, ev const &sp, int tp) {
    vector<NamedExp> v;
    for (int out_idx = 0; out_idx < num_ports; ++out_idx) {
        expr prev_turn = s[out_idx];
        ev backlogs_of_out_i = get_voq_of_out_i(b, out_idx);
        expr nxt_turn = rr(backlogs_of_out_i, prev_turn);
        v.emplace_back(sp[out_idx] == nxt_turn);
    }
    return v;
}

vector<NamedExp> LeafSts::init(const ev &b0, const ev &s0) {
    expr res = slv.ctx.bool_val(true);
    for (int i = 0; i < num_ports; ++i)
        res = res && (s0[i] == 0);

    return {res};
}

template<typename V>
V LeafSts::get_voq_of_out_i(const V &all_ev, const int i) {
    V ev;
    int dst = i;
    for (int src = 0; src < num_ports; ++src) {
        int idx = src * num_ports + dst;
        ev.push_back(all_ev[idx]);
    }
    return ev;
}

void LeafSts::print(model mod) const {
    for (int src = 0; src < num_ports; ++src) {
        cout << "------------------------" << endl;
        cout << "In[" << src << "]: " << endl;
        for (int dst = 0; dst < num_ports; ++dst) {
            auto idx = src * num_ports + dst;
            cout << "TO[" << dst << "]: " << str(I[idx], mod, ",").str() << endl;
        }
    }
    cout << endl;
    for (int dst = 0; dst < num_ports; ++dst) {
        cout << "------------------------" << endl;
        cout << "Out[" << dst << "]: " << endl;
        for (int src = 0; src < num_ports; ++src) {
            auto idx = src * num_ports + dst;
            cout << "FROM[" << src << "]: " << str(O[idx], mod, ",").str() << endl;
        }
    }
    cout << endl;
    for (int i = 0; i < num_ports; ++i) {
        cout << "------------------------" << endl;
        cout << "Turn Dst[" << i << "]" << endl;
        cout << str(get_state()[i], mod).str() << endl;
    }
    // for (int i = 0; i < num_ports; ++i) {
    // cout << "------------------------" << endl;
    // cout << "M = " << i << endl;
    // cout << str(I[i], mod, ",").str() << endl;
    // for (int j = 0; j < k; ++j) {
    // cout << str(I[i * k + j], mod, ",").str() << endl;
    // }
    // }
}

vector<NamedExp> LeafSts::inputs(const int i) {
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

vector<NamedExp> LeafSts::base_constrs() {
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


vector<NamedExp> LeafSts::bl_size(const int i) const {
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
    return res;
}

std::vector<NamedExp> LeafSts::enqs(const int i) const {
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


std::vector<NamedExp> LeafSts::drops(int i) {
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

std::vector<NamedExp> LeafSts::enq_deq_sum(int i) {
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


vector<NamedExp> LeafSts::winds(int i) {
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

ev2 LeafSts::get_state() const {
    return S_int;
}

vector<NamedExp> LeafSts::trs() {
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


vector<NamedExp> LeafSts::out() {
    vector<NamedExp> res;
    for (int j = 0; j < timesteps; ++j) {
        auto nes = out(get_buf_vec_at_i(B, j), get_buf_vec_at_i(get_state(), j), get_buf_vec_at_i(O, j), j);
        extend(res, nes, format("@{}", j));
    }
    return res;
    // return {merge(res, "out")};
}
