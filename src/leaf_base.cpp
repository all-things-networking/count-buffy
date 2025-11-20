//
// Created by Amir Hossein Seyhani on 10/28/25.
//

#include "leaf_base.hpp"

#include <map>
#include <ranges>

#include "Buff.hpp"
#include "prio_sts.hpp"
#include <set>

#include "SparseBuff.hpp"

using namespace views;

const int MAX_I = 10;

map<int, map<int, Buff *> > LeafBase::src_map_per_dst() {
    map<int, map<int, Buff *> > src_buffs_map;
    for (const auto &[key, buf]: buffs) {
        int src = get<0>(key);
        int dst = get<1>(key);
        auto buff = buffs[{src, dst}];
        src_buffs_map[dst][src] = buff;
    }
    return src_buffs_map;
}

map<int, map<int, Buff *> > LeafBase::dst_map_per_src() {
    map<int, map<int, Buff *> > dst_buffs_map;
    for (const auto &[key, buf]: buffs) {
        int src = get<0>(key);
        int dst = get<1>(key);
        auto buff = buffs[{src, dst}];
        dst_buffs_map[src][dst] = buff;
    }
    return dst_buffs_map;
}

ev2 LeafBase::get_in_port(int src) {
    vector<Buff *> src_buffs = get_buffs_for_src(src);
    assert(src_buffs.size() > 0);
    ev2 in = src_buffs[0]->getExpandedI();
    for (int i = 1; i < src_buffs.size(); ++i) {
        in = in + src_buffs[i]->getExpandedI();
    }
    return in;
}

vector<int> LeafBase::get_in_ports() {
    vector<int> in_ports;
    for (const auto &[src_dst, buff]: buffs) {
        int src = get<0>(src_dst);
        if (ranges::find(in_ports, src) == in_ports.end()) {
            in_ports.push_back(src);
        }
    }
    return in_ports;
}

vector<int> LeafBase::get_out_ports() {
    vector<int> out_ports;
    for (const auto &[src_dst, buff]: buffs) {
        int dst = get<0>(src_dst);
        if (ranges::find(out_ports, dst) == out_ports.end()) {
            out_ports.push_back(dst);
        }
    }
    return out_ports;
}

ev2 LeafBase::get_out_port(int dst) {
    vector<Buff *> buffs_of_dst = get_buffs_for_dst(dst);
    if (buffs_of_dst.empty())
        return slv.ivv(timesteps, pkt_types, 0);
    ev2 out = buffs_of_dst[0]->getExpandedO();
    for (int i = 1; i < buffs_of_dst.size(); ++i) {
        out = out + buffs_of_dst[i]->getExpandedO();
    }
    return out;
}


LeafBase::LeafBase(SmtSolver &slv, const string &var_prefix, vector<tuple<int, int> > port_list,
                   const int time_steps,
                   const int pkt_types,
                   const int buff_cap,
                   const int max_enq,
                   const int max_deq
) : slv(slv), var_prefix(move(var_prefix)),
    timesteps(time_steps), pkt_types(pkt_types), buff_cap(buff_cap), max_enq(max_enq),
    max_deq(max_deq) {
    for (auto src_dst: port_list) {
        int src = get<0>(src_dst);
        int dst = get<1>(src_dst);
        Buff *buff = new Buff(slv, format("{}_BUF_{}_{}", var_prefix, src, dst), timesteps,
                              pkt_types, max_enq, max_deq, buff_cap, src, dst);
        buffs[{src, dst}] = buff;
    }
    use_win = false;
}

LeafBase::LeafBase(SmtSolver &slv, const string &var_prefix, map<tuple<int, int>, vector<int> > port_list,
                   const int time_steps,
                   const int pkt_types,
                   const int buff_cap,
                   const int max_enq,
                   const int max_deq
) : slv(slv), var_prefix(move(var_prefix)),
    timesteps(time_steps), pkt_types(pkt_types), buff_cap(buff_cap), max_enq(max_enq),
    max_deq(max_deq) {
    for (auto src_dst_pkt_type: port_list) {
        auto src_dst = get<0>(src_dst_pkt_type);
        auto used_pkt_types = get<1>(src_dst_pkt_type);
        int src = get<0>(src_dst);
        int dst = get<1>(src_dst);
        if (used_pkt_types.empty())
            continue;
        Buff *buff = new Buff(slv, format("{}_BUF_{}_{}", var_prefix, src, dst), timesteps,
                              pkt_types, max_enq, max_deq, buff_cap, src, dst, used_pkt_types);
        buffs[{src, dst}] = buff;
    }
    use_win = false;
}

vector<Buff *> LeafBase::get_buff_list() const {
    vector<Buff *> result;
    result.reserve(buffs.size());
    for (const auto buffPtr: values(buffs))
        result.push_back(buffPtr);
    return result;
}

expr LeafBase::rr_for_dst(const vector<Buff *> &buffs, int t, expr prev_turn) {
    vector<int> src_vals;
    for (auto buff: buffs) {
        src_vals.push_back(buff->src);
    }

    int count = buffs.size();
    expr nxt_turn = prev_turn;
    for (int i = 0; i < count; ++i) {
        int src_i = src_vals[i];
        expr x = slv.ctx.int_val(src_i);
        for (int j = 1; j < count; ++j) {
            const int l = (i - j + count) % count;
            int src_j = src_vals[l];
            x = ite(buffs[l]->B[t], slv.ctx.int_val(src_j), x);
        }
        nxt_turn = ite(prev_turn == slv.ctx.int_val(src_i), x, nxt_turn);
    }
    return nxt_turn;
}

expr LeafBase::rr_for_src(const vector<Buff *> &buffs, int t, expr prev_turn) {
    vector<int> dst_vals;
    for (auto buff: buffs) {
        dst_vals.push_back(buff->dst);
    }

    int count = buffs.size();
    expr nxt_turn = prev_turn;
    for (int i = 0; i < count; ++i) {
        int dst_i = dst_vals[i];
        expr x = slv.ctx.int_val(dst_i);
        for (int j = 1; j < count; ++j) {
            const int l = (i - j + count) % count;
            int dst_j = dst_vals[l];
            x = ite(buffs[l]->B[t], slv.ctx.int_val(dst_j), x);
        }
        nxt_turn = ite(prev_turn == slv.ctx.int_val(dst_i), x, nxt_turn);
    }
    return nxt_turn;
}

vector<Buff *> LeafBase::get_buffs_for_dst(int dst) {
    auto per_dst = src_map_per_dst();
    auto dst_map = per_dst[dst];
    vector<Buff *> dst_buffs_list;
    for (const auto &[src, buff]: dst_map)
        dst_buffs_list.push_back(buff);
    return dst_buffs_list;
}

vector<Buff *> LeafBase::get_buffs_for_src(int src) {
    vector<Buff *> src_buffs;
    for (const auto &[key, buff]: buffs) {
        int s = get<0>(key);
        if (s == src)
            src_buffs.push_back(buff);
    }
    return src_buffs;
}

void LeafBase::print(model mod) {
    cout << var_prefix << endl << "######################################################################" << endl;
    for (const auto &[src_dst, buf]: buffs) {
        int src = get<0>(src_dst);
        int dst = get<1>(src_dst);
        cout << "--------------" << endl;
        cout << src << " -> " << dst << endl;
        cout << "IN :" << endl;
        cout << str(buf->getI(), mod, ",").str() << endl;
        cout << "OUT:" << endl;
        cout << str(buf->O, mod, ",").str() << endl;
        cout << "DST" << endl;
        cout << "BL :" << endl;
        cout << str(buf->B, mod).str() << endl;
        cout << "DST Turn SRC =  " << src << endl << str(dst_turn_for_src[src], mod).str() << endl;
        cout << "SRC Turn DST =  " << dst << endl << str(src_turn_for_dst[dst], mod).str() << endl;
        cout << "TMP Turn SRC =  " << src << endl << str(tmp_per_src[src], mod).str() << endl;
        cout << "TMP Turn DST =  " << dst << endl << str(tmp_per_dst[dst], mod).str() << endl;
        cout << "Match:" << endl;
        cout << str(matched[{src, dst}], mod).str() << endl;
        cout << endl;
    }


    // cout << "TMP: " << endl;
    // cout << str(tmp, mod).str() << endl;
    // for (const auto &[dst, turn]: turn_for_dst) {
    // auto src_buffs = get_buffs_for_dst(dst);
    // cout << "-----------------" << endl;
    // cout << "DST = " << dst << endl;
    // for (int i = 0; i < src_buffs.size(); ++i) {
    // auto buf = src_buffs[i];
    // cout << "IN: " << i << "-" << buf->src << endl;
    // cout << str(buf->I, mod, ",").str() << endl;
    // cout << "O : " << i << "-" << buf->src << endl;
    // cout << str(buf->O, mod, ",").str() << endl;
    // cout << "B : " << i << "-" << buf->src << endl;
    // cout << str(buf->B, mod).str() << endl;
    // }
    // expr turn_val = slv.ctx.int_val(0);
    // for (int i = 1; i <= src_buffs.size(); ++i) {
    //     int idx = ((src_buffs.size() - i) % src_buffs.size());
    //     turn_val = ite(src_buffs[idx]->B[0], slv.ctx.int_val(idx), turn_val);
    // }
    // res = res && (turn_for_dst[dst][0] == turn_val);
    // }
    // for (const auto &[dst, v]: turn_for_dst) {
    // cout << "-----------------------------" << endl;
    // cout << "DST: " << dst << endl;
    // cout << str(v, mod).str() << endl;
    // }
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

vector<NamedExp> LeafBase::winds_old(int i) {
    const auto Oi = get_buff_list()[i]->O;
    const auto Ei = get_buff_list()[i]->E;
    const auto wnd_enq_i = get_buff_list()[i]->wnd_enq;
    const auto wnd_out_i = get_buff_list()[i]->wnd_out;
    const auto wnd_enq_nxt_i = get_buff_list()[i]->wnd_enq_nxt;
    const auto tmp_wnd_enq_i = get_buff_list()[i]->tmp_wnd_enq;
    const auto tmp_wnd_enq_nxt_i = get_buff_list()[i]->tmp_wnd_enq_nxt;
    const auto Ci = get_buff_list()[i]->C;
    vector<NamedExp> nes;

    int cap = buff_cap;
    // [14]
    nes.emplace_back(wnd_enq_i[0] == Ei[0], format("WndEnq[{}]@{}", i, 0));
    nes.emplace_back(wnd_out_i[0] == Oi[0], format("WndOut[{}]@{}", i, 0));
    nes.emplace_back(wnd_enq_nxt_i[0] == 0, format("WndNxt[{}]@{}", i, 0));
    for (int j = 1; j < timesteps; ++j) {
        auto te = tmp_wnd_enq_i[j];
        auto tn = tmp_wnd_enq_nxt_i[j];
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
        ev total_sum = wnd_enq_i[j - 1] + wnd_enq_nxt_i[j - 1] + Ei[j];
        // auto te = ite(total_sum <= c, total_sum, c);


        nes.emplace_back((te + tn) == total_sum, format("Update te + tn[{}]@{}", i, j));
        // [16], [17], [18], [19]
        nes.emplace_back((!(te < cap && tn > 0)) && (te <= cap) && (tn <= cap) && (wnd_enq_i[j - 1] <= te),
                         format("Overflow mechanism[{}]@{}", i, j));
        // [20]
        // nes.emplace_back(to == (wnd_out[i][j - 1] + O[i][j]), format("Update to[{}]@{}", i, j));

        auto to = wnd_out_i[j - 1] + Oi[j];
        // [21]
        // nes.emplace_back(m == (te <= to), format("Match[{}]@{}", i, j));
        auto m = te <= to;
        // [22]
        nes.emplace_back(
            ite(m, wnd_enq_i[j] == tn, ite(total_sum <= cap, wnd_enq_i[j] == total_sum, wnd_enq_i[j] == total_sum)),
            format("Update WE[{}]@{}", i, j));
        // [23]
        nes.emplace_back(ite(m, wnd_enq_nxt_i[j] == 0, wnd_enq_nxt_i[j] == tn), format("Update WN[{}]@{}", i, j));
        // [24]
        nes.emplace_back(ite(m, wnd_out_i[j] == to - te, wnd_out_i[j] == to), format("Update WO[{}]@{}", i, j));
        // [25]
        nes.emplace_back(wnd_out_i[j] <= wnd_enq_i[j], format("WndOut <= WndEnq[{}]@{}", i, j));

        nes.emplace_back(Ci[j] == wnd_enq_i[j] + wnd_enq_nxt_i[j] - wnd_out_i[j], format("equity[{}]@{}", i, j));
    }
    return nes;
}

vector<NamedExp> LeafBase::inputs(const int i) {
    vector<NamedExp> res;
    extend(res, bl_size(i));
    extend(res, enqs(i));
    extend(res, drops(i));
    extend(res, enq_deq_sum(i));
    if (this->use_win) {
        // extend(res, winds_old(i));
        extend(res, winds(i));
    }
    return res;
}

vector<NamedExp> LeafBase::base_constrs() {
    vector<NamedExp> res;
    auto buffs_list = get_buff_list();
    for (int i = 0; i < buffs_list.size(); ++i) {
        if (buffs_list[i]->empty)
            continue;
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

vector<NamedExp> LeafBase::bl_size(const int i) const {
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
    return {merge(res, format("BL Size[{}]", i))};
}

vector<NamedExp> LeafBase::enqs(const int i) const {
    const auto Ei = get_buff_list()[i]->E;
    const auto Bi = get_buff_list()[i]->B;
    const auto Ci = get_buff_list()[i]->C;
    const auto Oi = get_buff_list()[i]->O;

    vector<NamedExp> res;

    // [9]
    expr e0 = (Ei[0] <= buff_cap) && (Bi[0] == (Ei[0] > 0));
    res.emplace_back(e0);

    for (int j = 1; j < timesteps; ++j) {
        // [10]
        expr lt_cap = ((Ei[j] + Ci[j - 1]) <= buff_cap);
        expr ej = lt_cap;
        res.emplace_back(ej);
    }
    return {merge(res, format("Enqs[{}]", i))};
    // return res;
}

vector<NamedExp> LeafBase::drops(int i) {
    const auto Di = get_buff_list()[i]->D;
    const auto Ei = get_buff_list()[i]->E;
    const auto Bi = get_buff_list()[i]->B;
    const auto Ci = get_buff_list()[i]->C;

    vector<NamedExp> res;

    expr d0 = ite(Bi[0], implies(Di[0] > 0, Ei[0] == buff_cap), Di[0] == 0);
    res.emplace_back(d0);

    for (int j = 1; j < timesteps; ++j) {
        expr dj = ite(Bi[j],
                      implies(Di[j] > 0, (Ci[j - 1] + Ei[j]) == buff_cap),
                      Di[j] == 0);
        res.emplace_back(dj);
    }
    return {merge(res, format("Drops[{}]", i))};
}

vector<NamedExp> LeafBase::enq_deq_sum(int i) {
    const auto Ii = get_buff_list()[i]->getI();
    const auto Di = get_buff_list()[i]->D;
    const auto Ei = get_buff_list()[i]->E;

    vector<NamedExp> res;

    res.emplace_back(Ii[0] == (Ei[0] + Di[0]));

    for (int j = 1; j < timesteps; ++j) {
        expr ij = (Ii[j] == (Ei[j] + Di[j]));
        res.emplace_back(ij);
    }
    return {merge(res, format("EnqDropSum[{}]", i))};
}

vector<NamedExp> LeafBase::winds(int i) {
    const auto Oi = get_buff_list()[i]->O;
    const auto Ei = get_buff_list()[i]->E;
    const auto wnd_enq_i = get_buff_list()[i]->wnd_enq;
    const auto wnd_out_i = get_buff_list()[i]->wnd_out;
    const auto wnd_enq_nxt_i = get_buff_list()[i]->wnd_enq_nxt;
    const auto tmp_wnd_enq_i = get_buff_list()[i]->tmp_wnd_enq;
    const auto tmp_wnd_enq_nxt_i = get_buff_list()[i]->tmp_wnd_enq_nxt;

    vector<NamedExp> nes;
    nes.emplace_back(wnd_enq_i[0] == Ei[0]);
    nes.emplace_back(wnd_out_i[0] == Oi[0]);
    nes.emplace_back(wnd_enq_nxt_i[0] == 0);
    for (int j = 1; j < timesteps; ++j) {
        const auto &se = tmp_wnd_enq_i[j];
        const auto &sn = tmp_wnd_enq_nxt_i[j];
        nes.emplace_back(wnd_enq_i[j - 1] + Ei[j] == se + sn);

        auto to = wnd_out_i[j - 1] + Oi[j];
        auto m = se <= to;
        auto constr = ite(m,
                          wnd_enq_i[j] == wnd_enq_nxt_i[j - 1] + sn
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
    return {merge(nes, format("wins[{}]", i))};
}

vector<NamedExp> LeafBase::trs() {
    vector<NamedExp> res;
    auto nes = init();
    extend(res, nes);
    for (int i = 0; i < timesteps - 1; ++i) {
        nes = trs(i);
        if (!nes.empty())
            res.push_back(merge(nes, format("Trs({},{})", i, i + 1)));
    }
    return res;
}

vector<NamedExp> LeafBase::out() {
    vector<NamedExp> res;
    for (int j = 0; j < timesteps; ++j) {
        auto nes = out(j);
        extend(res, nes, format("@{}", j));
    }
    return {merge(res, "out")};
}
