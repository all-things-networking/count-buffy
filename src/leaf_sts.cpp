#include "leaf_sts.hpp"

#include <map>

#include "Buff.hpp"
#include "prio_sts.hpp"

const int MAX_I = 10;

LeafSts::LeafSts(SmtSolver &slv, const string &var_prefix, const int num_bufs, const int time_steps,
                 const int pkt_types,
                 const int buff_cap,
                 const int max_enq,
                 const int max_deq,
                 const int num_ports
): slv(slv), var_prefix(move(var_prefix)), num_bufs(num_bufs),
   timesteps(time_steps), pkt_types(pkt_types), buff_cap(buff_cap), max_enq(max_enq),
   max_deq(max_deq), num_ports(num_ports) {
    for (int i = 0; i < num_bufs; ++i) {
        Buff buff(slv, format("BUF_{}", i), timesteps, pkt_types, max_enq, max_deq, buff_cap);
        buffs.push_back(buff);
    }
    use_win = true;
}

LeafSts::LeafSts(SmtSolver &slv, const string &var_prefix, int num_ports, int time_steps, int buff_cap, int max_enq,
                 int max_deq): LeafSts(slv, var_prefix, num_ports * num_ports, time_steps, 1, buff_cap, max_enq,
                                       max_deq,
                                       num_ports) {
}

vector<Buff> LeafSts::get_buff_list() const {
    return buffs;
}

vector<NamedExp> LeafSts::out(int t) {
    expr res = slv.ctx.bool_val(true);
    for (int src = 0; src < num_ports; ++src) {
        for (int dst = 0; dst < num_ports; ++dst) {
            int idx = src * num_ports + dst;
            // expr turn = sv[dst];
            // res = res && ite(bv[idx] && turn == slv.ctx.int_val(src), ov[idx] == 1, ov[idx] == 0);
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

vector<NamedExp> LeafSts::trs(int t) {
    vector<NamedExp> v;
    for (int out_idx = 0; out_idx < num_ports; ++out_idx) {
        // expr prev_turn = s[out_idx];
        // ev backlogs_of_out_i = get_voq_of_out_i(b, out_idx);
        // expr nxt_turn = rr(backlogs_of_out_i, prev_turn);
        // v.emplace_back(sp[out_idx] == nxt_turn);
    }
    return v;
}

vector<NamedExp> LeafSts::init() {
    expr res = slv.ctx.bool_val(true);
    // for (int i = 0; i < num_ports; ++i)
        // res = res && (s0[i] == 0);

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
            // cout << "TO[" << dst << "]: " << str(I[idx], mod, ",").str() << endl;
        }
    }
    cout << endl;
    for (int dst = 0; dst < num_ports; ++dst) {
        cout << "------------------------" << endl;
        cout << "Out[" << dst << "]: " << endl;
        for (int src = 0; src < num_ports; ++src) {
            auto idx = src * num_ports + dst;
            // cout << "FROM[" << src << "]: " << str(O[idx], mod, ",").str() << endl;
        }
    }
    cout << endl;
    for (int i = 0; i < num_ports; ++i) {
        cout << "------------------------" << endl;
        cout << "Turn Dst[" << i << "]" << endl;
        // cout << str(get_state()[i], mod).str() << endl;
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
    const auto Ei = get_buff_list()[i].E;
    const auto Bi = get_buff_list()[i].B;
    const auto Ci = get_buff_list()[i].C;
    const auto Oi = get_buff_list()[i].O;
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
    const auto Ei = get_buff_list()[i].E;
    const auto Bi = get_buff_list()[i].B;
    const auto Ci = get_buff_list()[i].C;
    const auto Oi = get_buff_list()[i].O;

    std::vector<NamedExp> res;

    // [9]
    expr e0 = (Ei[0] <= buff_cap) && (Bi[0] == (Ei[0] > 0));
    res.emplace_back(e0);

    for (int j = 1; j < timesteps; ++j) {
        // [10]
        expr lt_cap = ((Ei[j] + Ci[j - 1]) <= buff_cap);
        expr ej = lt_cap;
        res.emplace_back(ej);
    }
    return res;
}


std::vector<NamedExp> LeafSts::drops(int i) {
    const auto Di = get_buff_list()[i].D;
    const auto Ei = get_buff_list()[i].E;
    const auto Bi = get_buff_list()[i].B;
    const auto Ci = get_buff_list()[i].C;

    std::vector<NamedExp> res;

    // [11]
    expr d0 = ite(Bi[0], implies(Di[0] > 0, Ei[0] == buff_cap), Di[0] == 0);
    res.emplace_back(d0);

    // timesteps 1..m-1
    for (int j = 1; j < timesteps; ++j) {
        // [12]
        expr dj = ite(Bi[j],
                      implies(Di[j] > 0, (Ci[j - 1] + Ei[j]) == buff_cap),
                      Di[j] == 0);
        res.emplace_back(dj);
    }
    return res;
}

std::vector<NamedExp> LeafSts::enq_deq_sum(int i) {
    const auto Ii = get_buff_list()[i].I;
    const auto Di = get_buff_list()[i].D;
    const auto Ei = get_buff_list()[i].E;

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
    const auto Oi = get_buff_list()[i].O;
    const auto Ei = get_buff_list()[i].E;
    const auto wnd_enq_i = get_buff_list()[i].wnd_enq;
    const auto wnd_out_i = get_buff_list()[i].wnd_out;
    const auto wnd_enq_nxt_i = get_buff_list()[i].wnd_enq_nxt;
    const auto tmp_wnd_enq_i = get_buff_list()[i].tmp_wnd_enq;
    const auto tmp_wnd_enq_nxt_i = get_buff_list()[i].tmp_wnd_enq_nxt;

    vector<NamedExp> nes;
    // [14]
    nes.emplace_back(wnd_enq_i[0] == Ei[0]);
    nes.emplace_back(wnd_out_i[0] == Oi[0]);
    nes.emplace_back(wnd_enq_nxt_i[0] == 0);
    for (int j = 1; j < timesteps; ++j) {
        auto se = tmp_wnd_enq_i[j];
        auto sn = tmp_wnd_enq_nxt_i[j];
        nes.emplace_back(wnd_enq_i[j - 1] + Ei[j] == se + sn);

        auto to = wnd_out_i[j - 1] + Oi[j];
        auto m = se <= to;
        auto constr = ite(m,
                          wnd_enq_i[j] == wnd_enq_nxt_i[j] + sn
                          && wnd_enq_nxt_i[j] == slv.const_vec(pkt_types, 0)
                          && wnd_out_i[j] == to - se
                          ,
                          wnd_enq_i[j] == se
                          && wnd_enq_nxt_i[j] == wnd_enq_nxt_i[j - 1] + sn
                          && wnd_out_i[j] == to
        );
        constr = constr && se <= buff_cap && sn <= buff_cap - 1 && sn >= 0 && se >= 0 && to >= 0 && to <= 2 * buff_cap
                 && wnd_enq_nxt_i[j] <
                 buff_cap;
        constr = constr && (wnd_enq_i[j - 1] <= se);
        constr = constr && implies(sum(sn) > 0, sum(se) == buff_cap);
        constr = constr && implies(sum(se) < buff_cap, sum(sn) == 0);
        constr = constr && !(sum(se) < buff_cap && sum(sn) > 0);
        nes.emplace_back(constr);
        nes.emplace_back(wnd_out_i[j] <= wnd_enq_i[j]);
    }
    return nes;
}

vector<NamedExp> LeafSts::trs() {
    vector<NamedExp> res;
    auto nes = init();
    extend(res, nes);
    for (int i = 0; i < timesteps - 1; ++i) {
        nes = trs(i);
        extend(res, nes, format("Trs({},{})", i, i + 1));
    }
    return res;
}


vector<NamedExp> LeafSts::out() {
    vector<NamedExp> res;
    for (int j = 0; j < timesteps; ++j) {
        auto nes = out(j);
        extend(res, nes, format("@{}", j));
    }
    return res;
    // return {merge(res, "out")};
}
