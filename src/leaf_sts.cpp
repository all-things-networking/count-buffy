#include "leaf_sts.hpp"

#include <map>
#include <ranges>

#include "Buff.hpp"
#include "prio_sts.hpp"
#include <set>

using namespace views;

LeafSts::LeafSts(SmtSolver &slv, const string &var_prefix, vector<tuple<int, int> > port_list,
                 const int time_steps,
                 const int pkt_types,
                 const int buff_cap,
                 const int max_enq,
                 const int max_deq
) : LeafBase(slv, var_prefix, port_list, time_steps, pkt_types, buff_cap, max_enq, max_deq) {
}

LeafSts::LeafSts(SmtSolver &slv, const string &var_prefix, map<tuple<int, int>, vector<int> > port_list,
                 const int time_steps,
                 const int pkt_types,
                 const int buff_cap,
                 const int max_enq,
                 const int max_deq
) : LeafBase(slv, var_prefix, port_list, time_steps, pkt_types, buff_cap, max_enq, max_deq) {
}

vector<NamedExp> LeafSts::out(int t) {
    expr res = slv.ctx.bool_val(true);

    map<int, map<int, int> > dst_src_to_idx;
    map<int, map<int, int> > src_dst_to_idx;

    for (const auto &[key, buff]: buffs) {
        int src = get<0>(key);
        int dst = get<1>(key);
        if (buff->empty)
            continue;
        res = res && ite(
                  buff->B[t] && matched[{src, dst}][t], buff->O[t] == 1, buff->O[t] == 0
              );
    }
    return {res};
}

vector<NamedExp> LeafSts::trs(int t) {
    // Compute for t -> t+1
    auto per_dst = src_map_per_dst();
    auto per_src = dst_map_per_src();

    map<int, expr> tmp_src_turn_for_dst;
    map<int, expr> tmp_dst_turn_for_src;

    vector<NamedExp> v;
    for (const auto &[dst, dst_buffs_map]: per_dst) {
        vector<Buff *> dst_buffs_list;
        for (const auto &[src, buff]: dst_buffs_map)
            dst_buffs_list.push_back(buff);
        expr m = slv.ctx.bool_val(false);
        for (const auto &[src, buff]: dst_buffs_map) {
            m = m || matched[{src, dst}][t];
        }
        expr prev_turn = ite(m, src_turn_for_dst[dst][t], slv.ctx.int_val(dst_buffs_list[0]->src));
        auto nxt_turn_val = rr_for_dst(dst_buffs_list, t + 1, prev_turn);
        tmp_src_turn_for_dst.insert_or_assign(dst, nxt_turn_val);
    }

    for (const auto &[src, src_buffs_map]: per_src) {
        vector<Buff *> src_buffs_list;
        for (const auto &[src, buff]: src_buffs_map)
            src_buffs_list.push_back(buff);
        expr m = slv.ctx.bool_val(false);
        for (const auto &[dst, buff]: src_buffs_map) {
            m = m || matched[{src, dst}][t];
        }
        expr prev_turn = ite(m, dst_turn_for_src[src][t], slv.ctx.int_val(src_buffs_list[0]->dst));
        auto nxt_turn_val = rr_for_src(src_buffs_list, t + 1, prev_turn);
        tmp_dst_turn_for_src.insert_or_assign(src, nxt_turn_val);
    }


    for (const auto &[src, src_buffs_map]: per_src) {
        //     expr m = slv.ctx.bool_val(false);
        //     for (const auto &[dst, buff]: src_buffs_map) {
        //         m = m || matched[{src, dst}][t];
        //     }
        expr &cur_turn_for_src = tmp_dst_turn_for_src.at(src);
        //     expr &prev_turn_for_src = dst_turn_for_src[src][t];
        //     tmp_per_src[src].push_back(cur_turn_for_src);
        // dst_turn_for_src[src].push_back(ite(m, cur_turn_for_src, prev_turn_for_src));
        dst_turn_for_src[src].push_back(cur_turn_for_src);
    }

    for (const auto &[dst, dst_buffs_map]: per_dst) {
        //     expr m = slv.ctx.bool_val(false);
        //     for (const auto &[src, buff]: dst_buffs_map) {
        //         m = m || matched[{src, dst}][t];
        //     }
        expr &cur_turn_for_dst = tmp_src_turn_for_dst.at(dst);
        //     expr &prev_turn_for_dst = src_turn_for_dst[dst][t];
        //     tmp_per_dst[dst].push_back(cur_turn_for_dst);
        // src_turn_for_dst[dst].push_back(ite(m, cur_turn_for_dst, prev_turn_for_dst));
        src_turn_for_dst[dst].push_back(cur_turn_for_dst);
    }

    for (const auto &[key, buff]: buffs) {
        int src = get<0>(key);
        int dst = get<1>(key);

        expr match_uncase = buff->B[t + 1] && (
                                src_turn_for_dst.at(dst)[t + 1] == src && dst_turn_for_src.at(src)[t + 1] == dst
                            );

        matched[{src, dst}].push_back(match_uncase);
    }

    set<int> srcs;
    set<int> dsts;
    for (const auto &[key, buff]: buffs) {
        int src = get<0>(key);
        int dst = get<1>(key);
        srcs.insert(src);
        dsts.insert(dst);
    }

    // for (int src: srcs) {
    //     for (int d: dsts) {
    //         expr other_false = slv.ctx.bool_val(true);
    //         for (int dp: dsts) {
    //             if (d == dp)
    //                 continue;
    //             other_false = other_false && (!matched[{src, dp}][t + 1]);
    //         }
    //         expr e = implies(matched[{src, d}][t + 1], other_false);
    //         v.emplace_back(e);
    //     }
    // }
    //
    // for (int dst: dsts) {
    //     for (int s: srcs) {
    //         expr other_false = slv.ctx.bool_val(true);
    //         for (int sp: srcs) {
    //             if (s == sp)
    //                 continue;
    //             other_false = other_false && (!matched[{sp, dst}][t + 1]);
    //         }
    //         expr e = implies(matched[{s, dst}][t + 1], other_false);
    //         v.emplace_back(e);
    //     }
    // }


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
        expr x = slv.ctx.int_val(src_buffs_of_dst[0]->src);
        tmp_per_dst[dst].push_back(x);
        for (int j = 1; j <= count; ++j) {
            // int l = count - j;
            int l = j - 1;
            int val = src_buffs_of_dst[l]->src;
            x = ite(src_buffs_of_dst[l]->B[0], slv.ctx.int_val(val), x);
            tmp_per_dst[dst].push_back(x);
        }
        src_turn_for_dst[dst].push_back(x);
    }

    auto per_src = dst_map_per_src();
    map<int, expr> highest_prio_dst_for_src;
    for (const auto &[src, dst_map]: per_src) {
        vector<Buff *> dst_buffs_of_src;
        for (const auto &[dst, buff]: dst_map)
            dst_buffs_of_src.push_back(buff);

        int count = dst_buffs_of_src.size();
        expr x = slv.ctx.int_val(dst_buffs_of_src[0]->dst);
        tmp_per_src[src].push_back(x);
        for (int j = 1; j <= count; ++j) {
            // int l = (count - j);
            int l = j - 1;
            int val = dst_buffs_of_src[l]->dst;
            x = ite(dst_buffs_of_src[l]->B[0], slv.ctx.int_val(val), x);
            tmp_per_src[src].push_back(x);
        }
        dst_turn_for_src[src].push_back(x);
    }
    for (const auto &[key, buff]: buffs) {
        int src = get<0>(key);
        int dst = get<1>(key);
        expr m = buff->B[0] && src_turn_for_dst[dst][0] == src && dst_turn_for_src[src][0] == dst;
        matched[{src, dst}].push_back(m);
        //
        //
    }
    return v;
}
