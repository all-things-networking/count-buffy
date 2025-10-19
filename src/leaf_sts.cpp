#include "leaf_sts.hpp"

#include <map>
#include <ranges>

#include "Buff.hpp"
#include "prio_sts.hpp"
#include <set>

using namespace views;

const int MAX_I = 10;

map<int, map<int, Buff *> > LeafSts::src_map_per_dst() {
    map<int, map<int, Buff *> > src_buffs_map;
    for (const auto &[key, buf]: buffs) {
        int src = get<0>(key);
        int dst = get<1>(key);
        auto buff = buffs[{src, dst}];
        src_buffs_map[dst][src] = buff;
    }
    return src_buffs_map;
}

map<int, map<int, Buff *> > LeafSts::dst_map_per_src() {
    map<int, map<int, Buff *> > dst_buffs_map;
    for (const auto &[key, buf]: buffs) {
        int src = get<0>(key);
        int dst = get<1>(key);
        auto buff = buffs[{src, dst}];
        dst_buffs_map[src][dst] = buff;
    }
    return dst_buffs_map;
}

ev2 LeafSts::get_in_port(int src) {
    vector<Buff *> src_buffs = get_buffs_for_src(src);
    assert(src_buffs.size() > 0);
    ev2 in = src_buffs[0]->I;
    for (int i = 1; i < src_buffs.size(); ++i) {
        in = in + src_buffs[i]->I;
    }
    return in;
}

vector<int> LeafSts::get_in_ports() {
    vector<int> in_ports;
    for (const auto &[src_dst, buff]: buffs) {
        int src = get<0>(src_dst);
        if (ranges::find(in_ports, src) == in_ports.end()) {
            in_ports.push_back(src);
        }
    }
    return in_ports;
}

vector<int> LeafSts::get_out_ports() {
    vector<int> out_ports;
    for (const auto &[src_dst, buff]: buffs) {
        int dst = get<0>(src_dst);
        if (ranges::find(out_ports, dst) == out_ports.end()) {
            out_ports.push_back(dst);
        }
    }
    return out_ports;
}

ev2 LeafSts::get_out_port(int dst) {
    vector<Buff *> dst_buffs = get_buffs_for_dst(dst);
    assert(dst_buffs.size() > 0);
    ev2 out = dst_buffs[0]->O;
    for (int i = 1; i < dst_buffs.size(); ++i) {
        out = out + dst_buffs[i]->O;
    }
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
    // set<int> dsts;
    // set<int> srcs;

    for (auto src_dst: port_list) {
        int src = get<0>(src_dst);
        int dst = get<1>(src_dst);
        Buff *buff = new Buff(slv, format("{}_BUF_{}_{}", var_prefix, src, dst), timesteps,
                              pkt_types, max_enq, max_deq, buff_cap, src, dst);
        buffs[{src, dst}] = buff;
        // dsts.insert(dst);
        // srcs.insert(src);
    }

    // for (int dst: dsts) {
    // src_turn_for_dst[dst] = slv.iv(time_steps, format("TURN_DST_{}", dst));
    // }

    // for (int src: srcs) {
    // dst_turn_for_src[src] = slv.iv(time_steps, format("TURN_SRC_{}", src));
    // }
    use_win = true;
}

vector<Buff *> LeafSts::get_buff_list() const {
    vector<Buff *> result;
    result.reserve(buffs.size());
    for (const auto buffPtr: values(buffs))
        result.push_back(buffPtr);
    return result;
}

vector<NamedExp> LeafSts::out(int t) {
    expr res = slv.ctx.bool_val(true);

    map<int, map<int, int> > dst_src_to_idx;
    map<int, map<int, int> > src_dst_to_idx;

    auto per_dst = src_map_per_dst();
    for (const auto &[dst, src_map]: per_dst) {
        vector<Buff *> src_buffs_list;
        for (const auto &[src, buff]: src_map)
            src_buffs_list.push_back(buff);
        for (int i = 0; i < src_buffs_list.size(); ++i) {
            auto buff = src_buffs_list[i];
            dst_src_to_idx[dst][buff->src] = i;
        }
    }

    auto per_src = dst_map_per_src();
    for (const auto &[src, dst_map]: per_src) {
        vector<Buff *> dst_buffs_list;
        for (const auto &[dst, buff]: dst_map)
            dst_buffs_list.push_back(buff);
        for (int i = 0; i < dst_buffs_list.size(); ++i) {
            auto buff = dst_buffs_list[i];
            src_dst_to_idx[src][buff->dst] = i;
        }
    }

    // for (const auto &[key, buff]: buffs) {
    //     int src = get<0>(key);
    //     int dst = get<1>(key);
    //
    //     int src_idx_for_dst = dst_src_to_idx[dst][src];
    //     int dst_idx_for_src = src_dst_to_idx[src][dst];
    //
    //     expr sel_dst_idx_for_src = selected_dst_idx_for_src[src][t];
    //     expr sel_src_idx_for_dst = selected_src_idx_for_dst[dst][t];
    //
    //     res = res && ite(
    //               buff->B[t] && (sel_dst_idx_for_src == slv.ctx.int_val(dst_idx_for_src)) && (
    //                   sel_src_idx_for_dst == slv.ctx.int_val(src_idx_for_dst)),
    //               buff->O[t] == 1, buff->O[t] == 0);
    // }

    for (const auto &[key, buff]: buffs) {
        int src = get<0>(key);
        int dst = get<1>(key);
        //
        //     int src_idx_for_dst = dst_src_to_idx[dst][src];
        //     int dst_idx_for_src = src_dst_to_idx[src][dst];
        //
        //     expr sel_dst_idx_for_src = selected_dst_idx_for_src[src][t];
        //     expr sel_src_idx_for_dst = selected_src_idx_for_dst[dst][t];
        //
        res = res && ite(
                  buff->B[t] && matched[{src, dst}][t], buff->O[t] == 1, buff->O[t] == 0
              );
        // buff->B[t] && dst_turn_for_src[src][t] == dst
        // && src_turn_for_dst[dst][t] == src,
        //               buff->B[t] && (sel_dst_idx_for_src == slv.ctx.int_val(dst_idx_for_src)) && (
        //                   sel_src_idx_for_dst == slv.ctx.int_val(src_idx_for_dst)),
        // buff->O[t] == 1, buff->O[t] == 0);
        // }


        // for (const auto &[dst, turn]: turn_for_dst) {
        // vector<Buff *> src_buffs = get_buffs_for_dst(dst);
        // for (int i = 0; i < src_buffs.size(); ++i) {
        // Buff *buff = src_buffs[i];
        // res = res && ite(buff->B[t] && (turn[t] == slv.ctx.int_val(i)), buff->O[t] == 1, buff->O[t] == 0);
        // res = res && (buff->O[t] == 0);
    }
    // }
    return {res};
}

expr LeafSts::rr_for_dst(const vector<Buff *> &buffs, int t, int dst) {
    vector<int> src_vals;
    for (auto buff: buffs) {
        src_vals.push_back(buff->src);
    }

    int count = buffs.size();
    expr prev_turn = src_turn_for_dst[dst][t - 1];
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

expr LeafSts::rr_for_src(const vector<Buff *> &buffs, int t, int src) {
    vector<int> dst_vals;
    for (auto buff: buffs) {
        dst_vals.push_back(buff->dst);
    }

    int count = buffs.size();
    expr prev_turn = dst_turn_for_src[src][t - 1];
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

vector<Buff *> LeafSts::get_buffs_for_dst(int dst) {
    auto per_dst = src_map_per_dst();
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
    auto per_dst = src_map_per_dst();
    auto per_src = dst_map_per_src();

    map<int, expr> highest_prio_src_for_dst;
    map<int, expr> highest_prio_dst_for_src;

    map<int, expr> tmp_src_turn_for_dst;
    map<int, expr> tmp_dst_turn_for_src;

    vector<NamedExp> v;
    for (const auto &[dst, dst_buffs_map]: per_dst) {
        auto prev_turn = src_turn_for_dst[dst][t];
        vector<Buff *> dst_buffs_list;
        for (const auto &[src, buff]: dst_buffs_map)
            dst_buffs_list.push_back(buff);
        auto nxt_turn_val = rr_for_dst(dst_buffs_list, t + 1, dst);
        // tmp_src_for_dst.insert(dst, nxt_turn_val);
        // tmp_src_for_dst[dst] = nxt_turn_val;
        tmp_src_turn_for_dst.insert_or_assign(dst, nxt_turn_val);
        // src_turn_for_dst[dst].push_back(nxt_turn_val);
        // highest_prio_src_for_dst.insert_or_assign(dst, nxt_turn_val);
        // v.emplace_back(selected_src_idx_for_dst[dst][t + 1] == nxt_turn_val);
        // tmp_per_dst[dst].push_back(nxt_turn_val);
    }

    for (const auto &[src, src_buffs_map]: per_src) {
        auto prev_turn = dst_turn_for_src[src][t];
        vector<Buff *> src_buffs_list;
        for (const auto &[src, buff]: src_buffs_map)
            src_buffs_list.push_back(buff);
        auto nxt_turn_val = rr_for_src(src_buffs_list, t + 1, src);
        // tmp_dst_for_src[src] = nxt_turn_val;
        // highest_prio_dst_for_src.insert_or_assign(src, nxt_turn_val);
        tmp_dst_turn_for_src.insert_or_assign(src, nxt_turn_val);
        // dst_turn_for_src[src].push_back(nxt_turn_val);
        // v.emplace_back(selected_dst_idx_for_src[src][t + 1] == nxt_turn_val);
        // tmp_per_src[src].push_back(nxt_turn_val);
    }


    for (const auto &[src, src_buffs_map]: per_src) {
        expr m = slv.ctx.bool_val(false);
        for (const auto &[dst, buff]: src_buffs_map) {
            m = m || matched[{src, dst}][t];
        }
        expr &cur_turn_for_src = tmp_dst_turn_for_src.at(src);
        expr &prev_turn_for_src = dst_turn_for_src[src][t];
        tmp_per_src[src].push_back(cur_turn_for_src);
        dst_turn_for_src[src].push_back(ite(m, cur_turn_for_src, prev_turn_for_src));
    }

    for (const auto &[dst, dst_buffs_map]: per_dst) {
        expr m = slv.ctx.bool_val(false);
        for (const auto &[src, buff]: dst_buffs_map) {
            m = m || matched[{src, dst}][t];
        }
        expr &cur_turn_for_dst = tmp_src_turn_for_dst.at(dst);
        expr &prev_turn_for_dst = src_turn_for_dst[dst][t];
        tmp_per_dst[dst].push_back(cur_turn_for_dst);
        src_turn_for_dst[dst].push_back(ite(m, cur_turn_for_dst, prev_turn_for_dst));
    }

    for (const auto &[key, buff]: buffs) {
        int src = get<0>(key);
        int dst = get<1>(key);
        // expr &cur_turn_for_src = tmp_dst_turn_for_src.at(src);
        // expr &prev_turn_for_src = dst_turn_for_src[src][t];

        // expr &cur_turn_for_dst = tmp_src_turn_for_dst.at(dst);
        // expr &prev_turn_for_dst = src_turn_for_dst[dst][t];

        expr match = buff->B[t + 1]
                     && dst_turn_for_src.at(src)[t + 1] == dst
                     && src_turn_for_dst.at(dst)[t + 1] == src;
        matched[{src, dst}].push_back(match);
        // dst_turn_for_src[src].push_back(ite(match, cur_turn_for_src, prev_turn_for_src));
        // src_turn_for_dst[dst].push_back(ite(match, cur_turn_for_dst, prev_turn_for_dst));
        // expr e1 = (matched_dst_for_src[src][t + 1] == ite(match, highest_prio_dst_for_src.at(src),
        //                                                   matched_dst_for_src[src][t]));
        // v.emplace_back(e1);
        // expr e2 = (matched_src_for_dst[dst][t + 1] == ite(match, highest_prio_src_for_dst.at(dst),
        //                                                   matched_src_for_dst[dst][t]));
        // v.emplace_back(e2);
    }

    return v;
}

vector<NamedExp> LeafSts::init() {
    vector<NamedExp> v;

    auto per_dst = src_map_per_dst();
    for (const auto &[dst, src_map]: per_dst) {
        vector<Buff *> src_buffs_of_dst;
        for (const auto &[src, buff]: src_map)
            src_buffs_of_dst.push_back(buff);

        int count = src_buffs_of_dst.size();
        expr x = slv.ctx.int_val(src_buffs_of_dst[count - 1]->src);
        for (int j = 2; j <= count; ++j) {
            int l = count - j;
            int val = src_buffs_of_dst[l]->src;
            x = ite(src_buffs_of_dst[l]->B[0], slv.ctx.int_val(val), x);
        }
        src_turn_for_dst[dst].push_back(x);
        tmp_per_dst[dst].push_back(x);
    }

    auto per_src = dst_map_per_src();
    map<int, expr> highest_prio_dst_for_src;
    for (const auto &[src, dst_map]: per_src) {
        vector<Buff *> dst_buffs_of_src;
        for (const auto &[dst, buff]: dst_map)
            dst_buffs_of_src.push_back(buff);

        int count = dst_buffs_of_src.size();
        expr x = slv.ctx.int_val(dst_buffs_of_src[count - 1]->dst);
        for (int j = 2; j <= count; ++j) {
            int l = (count - j);
            int val = dst_buffs_of_src[l]->dst;
            x = ite(dst_buffs_of_src[l]->B[0], slv.ctx.int_val(val), x);
        }
        dst_turn_for_src[src].push_back(x);
        tmp_per_src[src].push_back(x);
    }

    // for (const auto &[src, dst_map]: per_src) {
    // expr e1 = (matched_dst_for_src[src][0] == highest_prio_dst_for_src.at(src));
    // per_src.insert_or_assign(src, highest_prio_dst_for_src.at(src));
    // tmp_per_src[src].push_back(highest_prio_dst_for_src.at(src));
    // v.emplace_back(e1, format("Highest Prio Dst for Src=[{}] @0", src));
    // }

    // for (const auto &[dst, dst_map]: per_dst) {
    // expr e2 = (src_turn_for_dst[dst][0] == highest_prio_src_for_dst.at(dst));
    // per_dst.insert_or_assign(dst, highest_prio_src_for_dst.at(dst));
    // tmp_per_dst[dst].push_back(highest_prio_src_for_dst.at(dst));
    // v.emplace_back(e2);
    // }
    //
    //
    for (const auto &[key, buff]: buffs) {
        int src = get<0>(key);
        int dst = get<1>(key);
        expr m = buff->B[0] && src_turn_for_dst[dst][0] == src && dst_turn_for_src[src][0] == dst;
        matched[{src, dst}].push_back(m);
        //
        //
    }
    return v;


    // map<int, map<int, int> > dst_src_to_idx;
    // map<int, map<int, int> > src_dst_to_idx;
    //
    // auto per_dst = get_per_dst_buff_map();
    // auto per_src = get_per_src_buff_map();
    //
    // map<int, expr> tmp_src_for_dst;
    // map<int, expr> tmp_dst_for_src;
    //
    // vector<NamedExp> v;
    // for (const auto &[key, buff]: buffs) {
    //     int dst = get<1>(key);
    //     auto prev_turn = selected_src_idx_for_dst[dst][t];
    //     auto dst_buffs = per_dst[dst];
    //     vector<Buff *> dst_buffs_list;
    //     for (const auto &[src, buff]: dst_buffs)
    //         dst_buffs_list.push_back(buff);
    //
    //     int count = dst_buffs_list.size();
    //     expr nxt_turn = slv.ctx.int_val(0);
    //     expr x = slv.ctx.int_val(-1);
    //     for (int j = 0; j < count; ++j) {
    //         int val = dst_buffs_list[j]->src;
    //         x = ite(buffs[l]->B[t], slv.ctx.int_val(val), x);
    //     }
    //     nxt_turn = ite(prev_turn == slv.ctx.int_val(vals[i]), x, nxt_turn);
    //     auto nxt_turn_val = rr(dst_buffs_list, prev_turn, 0, true);
    //     // tmp_src_for_dst.insert(dst, nxt_turn_val);
    //     // tmp_src_for_dst[dst] = nxt_turn_val;
    //     tmp_src_for_dst.insert_or_assign(dst, nxt_turn_val);
    //     // v.emplace_back(selected_src_idx_for_dst[dst][t + 1] == nxt_turn_val);
    // }
    //
    // for (const auto &[key, buff]: buffs) {
    //     int src = get<0>(key);
    //     auto prev_turn = selected_dst_idx_for_src[src][t];
    //     auto src_buffs = per_src[src];
    //     vector<Buff *> src_buffs_list;
    //     for (const auto &[src, buff]: src_buffs)
    //         src_buffs_list.push_back(buff);
    //     auto nxt_turn_val = rr(src_buffs_list, prev_turn, t + 1, false);
    //     // tmp_dst_for_src[src] = nxt_turn_val;
    //     tmp_dst_for_src.insert_or_assign(src, nxt_turn_val);
    //     // v.emplace_back(selected_dst_idx_for_src[src][t + 1] == nxt_turn_val);
    // }
    //
    // for (const auto &[key, buff]: buffs) {
    //     int src = get<0>(key);
    //     int dst = get<1>(key);
    //     expr match = buff->B[t]
    //                  && tmp_dst_for_src.at(src) == dst
    //                  && tmp_src_for_dst.at(dst) == src;
    //     expr e1 = (selected_dst_idx_for_src[src][t + 1] == ite(match, tmp_dst_for_src.at(src),
    //                                                            selected_dst_idx_for_src[src][t]));
    //     v.emplace_back(e1);
    //     expr e2 = (selected_src_idx_for_dst[dst][t + 1] == ite(match, tmp_src_for_dst.at(dst),
    //                                                            selected_src_idx_for_dst[dst][t]));
    //     v.emplace_back(e2);
    // }


    //
    // for (const auto &[dst, turn]: selected_src_idx_for_dst) {
    //     vector<Buff *> src_buffs = get_buffs_for_src(dst);
    //     vector<int> src_vals;
    //     for (auto buff: src_buffs) {
    //         src_vals.push_back(buff->src);
    //     }
    //     expr turn_val = slv.ctx.int_val(0);
    //     for (int i = 1; i <= src_buffs.size(); ++i) {
    //         int idx = ((src_buffs.size() - i) % src_buffs.size());
    //         int val = src_vals[idx];
    //         turn_val = ite(src_buffs[idx]->B[0], slv.ctx.int_val(val), turn_val);
    //     }
    //     res = res && (selected_src_idx_for_dst[dst][0] == turn_val);
    // }
    //
    //
    // for (const auto &[src, turn]: selected_dst_idx_for_src) {
    //     vector<Buff *> dst_buffs = get_buffs_for_src(src);
    //     vector<int> dst_vals;
    //     for (auto buff: dst_buffs) {
    //         dst_vals.push_back(buff->dst);
    //     }
    //     expr turn_val = slv.ctx.int_val(0);
    //     for (int i = 1; i <= dst_buffs.size(); ++i) {
    //         int idx = ((dst_buffs.size() - i) % dst_buffs.size());
    //         int val = dst_vals[idx];
    //         turn_val = ite(dst_buffs[idx]->B[0], slv.ctx.int_val(val), turn_val);
    //     }
    //     res = res && (selected_dst_idx_for_src[src][0] == turn_val);
    // }
    // return {res};
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
    cout << var_prefix << endl << "######################################################################" << endl;
    for (const auto &[src_dst, buf]: buffs) {
        int src = get<0>(src_dst);
        int dst = get<1>(src_dst);
        cout << "--------------" << endl;
        cout << src << " -> " << dst << endl;
        cout << "IN :" << endl;
        cout << str(buf->I, mod, ",").str() << endl;
        cout << "OUT:" << endl;
        cout << str(buf->O, mod, ",").str() << endl;
        cout << "DST" << endl;
        // cout << "BL :" << endl;
        // cout << str(buf->B, mod).str() << endl;
        // cout << "DST Turn SRC =  " << src << endl << str(dst_turn_for_src[src], mod).str() << endl;
        // cout << "SRC Turn DST =  " << dst << endl << str(src_turn_for_dst[dst], mod).str() << endl;
        // cout << "TMP Turn SRC =  " << src << endl << str(tmp_per_src[src], mod).str() << endl;
        // cout << "TMP Turn DST =  " << dst << endl << str(tmp_per_dst[dst], mod).str() << endl;
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
    return {merge(res, format("BL Size[{}]", i))};
}

vector<NamedExp> LeafSts::enqs(const int i) const {
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

vector<NamedExp> LeafSts::drops(int i) {
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

vector<NamedExp> LeafSts::enq_deq_sum(int i) {
    const auto Ii = get_buff_list()[i]->I;
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

vector<NamedExp> LeafSts::winds(int i) {
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

vector<NamedExp> LeafSts::trs() {
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

vector<NamedExp> LeafSts::out() {
    vector<NamedExp> res;
    for (int j = 0; j < timesteps; ++j) {
        auto nes = out(j);
        extend(res, nes, format("@{}", j));
    }
    return {merge(res, "out")};
}
