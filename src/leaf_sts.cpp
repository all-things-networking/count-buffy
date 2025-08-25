#include "leaf_sts.hpp"

#include <map>

#include "Buff.hpp"
#include "prio_sts.hpp"
#include <set>

const int MAX_I = 10;

map<int, map<int, Buff *> > LeafSts::get_per_dst_buff_map() {
    map<int, map<int, Buff *> > src_buffs_map;
    for (const auto &[key, buf]: buffs) {
        int src = get<0>(key);
        int dst = get<1>(key);
        auto buff = buffs[{src, dst}];
        src_buffs_map[dst][src] = buff;
    }
    return src_buffs_map;
}

ev2 LeafSts::get_in_port(int src) {
    vector<Buff *> src_buffs = get_buffs_for_src(src);
    assert(src_buffs.size() > 0);
    ev2 in = src_buffs[0]->I;
    for (auto buff: src_buffs)
        in = in + buff->I;
    return in;
}

ev2 LeafSts::get_out_port(int dst) {
    vector<Buff *> dst_buffs = get_buffs_for_dst(dst);
    assert(dst_buffs.size() > 0);
    ev2 out = dst_buffs[0]->O;
    for (auto buff: dst_buffs)
        out = out + buff->O;
    return out;
}


LeafSts::LeafSts(SmtSolver &slv, const string &var_prefix, vector<tuple<int, int> > port_list,
                 const int time_steps,
                 const int pkt_types,
                 const int buff_cap,
                 const int max_enq,
                 const int max_deq
): slv(slv), var_prefix(move(var_prefix)),
   timesteps(time_steps), pkt_types(pkt_types), buff_cap(buff_cap), max_enq(max_enq),
   max_deq(max_deq) {
    set<int> dsts;
    vector<Buff *> bar;
    tmp = slv.iv(time_steps, "TMP");

    for (auto src_dst: port_list) {
        int src = get<0>(src_dst);
        int dst = get<1>(src_dst);
        Buff *buff = new Buff(slv, format("{}_BUF_{}_{}", var_prefix, src, dst), timesteps,
                              pkt_types, max_enq, max_deq, buff_cap, src, dst);
        buffs[{src, dst}] = buff;
        dsts.insert(dst);
    }

    for (int dst: dsts) {
        turn_for_dst[dst] = slv.iv(time_steps, format("TURN_{}", dst));
    }
    use_win = true;
}

vector<Buff *> LeafSts::get_buff_list() const {
    std::vector<Buff *> result;
    for (const auto &[key, buffPtr]: buffs) {
        result.push_back(buffPtr);
    }
    return result;
}

vector<NamedExp> LeafSts::out(int t) {
    expr res = slv.ctx.bool_val(true);
    for (const auto &[dst, turn]: turn_for_dst) {
        vector<Buff *> src_buffs = get_buffs_for_dst(dst);
        expr turn_val = slv.ctx.int_val(0);
        for (int i = 0; i < src_buffs.size(); ++i) {
            Buff *buff = src_buffs[i];
            res = res && ite(buff->B[t] && (turn[t] == slv.ctx.int_val(i)), buff->O[t] == 1, buff->O[t] == 0);
            // res = res && (buff->O[t] == 1);
        }
    }
    return {res};
}

expr LeafSts::rr(const vector<Buff *> &src_buffs, const expr &prev_turn, int t) {
    int count = src_buffs.size();
    expr nxt_turn = slv.ctx.int_val(0);
    for (int i = 0; i < count; ++i) {
        expr x = slv.ctx.int_val(i);
        for (int j = 1; j < count; ++j) {
            const int l = (i - j + count) % count;
            x = ite(src_buffs[l]->B[t], slv.ctx.int_val(l), x);
        }
        nxt_turn = ite(prev_turn == i, x, nxt_turn);
    }
    return nxt_turn;
}

vector<Buff *> LeafSts::get_buffs_for_dst(int dst) {
    auto per_dst = get_per_dst_buff_map();
    auto dst_map = per_dst[dst];
    vector<Buff *> dst_buffs_list;
    for (const auto &[src, buff]: dst_map)
        dst_buffs_list.push_back(buff);
    return dst_buffs_list;
}

vector<Buff *> LeafSts::get_buffs_for_src(int src) {
    vector<Buff *> src_buffs;
    for (const auto &[key, buff]: buffs) {
        int s = get<0>(key);
        if (s == src)
            src_buffs.push_back(buff);
    }
    return src_buffs;
}


vector<NamedExp> LeafSts::trs(int t) {
    vector<NamedExp> v;
    auto per_dst = get_per_dst_buff_map();
    for (const auto &[key, buff]: buffs) {
        int src = get<0>(key);
        int dst = get<1>(key);
        auto prev_turn = turn_for_dst[dst][t];
        auto dst_buffs = per_dst[dst];
        vector<Buff *> dst_buffs_list;
        for (const auto &[src, buff]: dst_buffs)
            dst_buffs_list.push_back(buff);
        auto nxt_turn_val = rr(dst_buffs_list, prev_turn, t + 1);
        v.emplace_back(turn_for_dst[dst][t + 1] == nxt_turn_val);
        // v.emplace_back(turn_for_dst[dst][t + 1] == 7);
    }
    return v;
}

vector<NamedExp> LeafSts::init() {
    expr res = slv.ctx.bool_val(true);
    for (const auto &[dst, turn]: turn_for_dst) {
        vector<Buff *> src_buffs = get_buffs_for_dst(dst);
        expr turn_val = slv.ctx.int_val(0);
        for (int i = 1; i <= src_buffs.size(); ++i) {
            int idx = ((src_buffs.size() - i) % src_buffs.size());
            turn_val = ite(src_buffs[idx]->B[0], slv.ctx.int_val(idx), turn_val);
        }
        res = res && (turn_for_dst[dst][0] == turn_val);
    }
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

void LeafSts::print(model mod) {
    // for (const auto &[src_dst, buf]: buffs) {
    //     int src = get<0>(src_dst);
    //     int dst = get<1>(src_dst);
    //     cout << "-----------------------------" << endl;
    //     cout << src << " -> " << dst << endl;
    //     cout << "IN :" << endl;
    //     cout << str(buf->I, mod, ",").str() << endl;
    //     cout << "OUT:" << endl;
    //     cout << str(buf->O, mod, ",").str() << endl;
    // }

    cout << "TMP: " << endl;
    cout << str(tmp, mod).str() << endl;
    for (const auto &[dst, turn]: turn_for_dst) {
        auto src_buffs = get_buffs_for_dst(dst);
        cout << "-----------------" << endl;
        cout << "DST = " << dst << endl;
        for (int i = 0; i < src_buffs.size(); ++i) {
            auto buf = src_buffs[i];
            cout << "IN: " << i << "-" << buf->src << endl;
            cout << str(buf->I, mod, ",").str() << endl;
            cout << "O : " << i << "-" << buf->src << endl;
            cout << str(buf->O, mod, ",").str() << endl;
            cout << "B : " << i << "-" << buf->src << endl;
            cout << str(buf->B, mod).str() << endl;
        }
        // expr turn_val = slv.ctx.int_val(0);
        // for (int i = 1; i <= src_buffs.size(); ++i) {
        //     int idx = ((src_buffs.size() - i) % src_buffs.size());
        //     turn_val = ite(src_buffs[idx]->B[0], slv.ctx.int_val(idx), turn_val);
        // }
        // res = res && (turn_for_dst[dst][0] == turn_val);
    }
    for (const auto &[dst, v]: turn_for_dst) {
        cout << "-----------------------------" << endl;
        cout << "DST: " << dst << endl;
        cout << str(v, mod).str() << endl;
    }
    // int src = get<0>(src_dst);
    // int dst = get<1>();
    // for (int src = 0; src < num_ports; ++src) {
    //     cout << "------------------------" << endl;
    //     cout << "In[" << src << "]: " << endl;
    //     for (int dst = 0; dst < num_ports; ++dst) {
    //         auto idx = src * num_ports + dst;
    //         // cout << "TO[" << dst << "]: " << str(I[idx], mod, ",").str() << endl;
    //     }
    // }
    // cout << endl;
    // for (int dst = 0; dst < num_ports; ++dst) {
    //     cout << "------------------------" << endl;
    //     cout << "Out[" << dst << "]: " << endl;
    //     for (int src = 0; src < num_ports; ++src) {
    //         auto idx = src * num_ports + dst;
    //         // cout << "FROM[" << src << "]: " << str(O[idx], mod, ",").str() << endl;
    //     }
    // }
    // cout << endl;
    // for (int i = 0; i < num_ports; ++i) {
    //     cout << "------------------------" << endl;
    //     cout << "Turn Dst[" << i << "]" << endl;
    //     // cout << str(get_state()[i], mod).str() << endl;
    // }
    // // for (int i = 0; i < num_ports; ++i) {
    // // cout << "------------------------" << endl;
    // // cout << "M = " << i << endl;
    // // cout << str(I[i], mod, ",").str() << endl;
    // // for (int j = 0; j < k; ++j) {
    // // cout << str(I[i * k + j], mod, ",").str() << endl;
    // // }
    // // }
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
    for (int i = 0; i < buffs.size(); ++i) {
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
    const auto Ei = get_buff_list()[i]->E;
    const auto Bi = get_buff_list()[i]->B;
    const auto Ci = get_buff_list()[i]->C;
    const auto Oi = get_buff_list()[i]->O;
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
    const auto Ei = get_buff_list()[i]->E;
    const auto Bi = get_buff_list()[i]->B;
    const auto Ci = get_buff_list()[i]->C;
    const auto Oi = get_buff_list()[i]->O;

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
    const auto Di = get_buff_list()[i]->D;
    const auto Ei = get_buff_list()[i]->E;
    const auto Bi = get_buff_list()[i]->B;
    const auto Ci = get_buff_list()[i]->C;

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
    const auto Ii = get_buff_list()[i]->I;
    const auto Di = get_buff_list()[i]->D;
    const auto Ei = get_buff_list()[i]->E;

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
    const auto Oi = get_buff_list()[i]->O;
    const auto Ei = get_buff_list()[i]->E;
    const auto wnd_enq_i = get_buff_list()[i]->wnd_enq;
    const auto wnd_out_i = get_buff_list()[i]->wnd_out;
    const auto wnd_enq_nxt_i = get_buff_list()[i]->wnd_enq_nxt;
    const auto tmp_wnd_enq_i = get_buff_list()[i]->tmp_wnd_enq;
    const auto tmp_wnd_enq_nxt_i = get_buff_list()[i]->tmp_wnd_enq_nxt;

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
        res.push_back(merge(nes, format("Trs({},{})", i, i + 1)));
        // extend(res, nes, format("Trs({},{})", i, i + 1));
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
