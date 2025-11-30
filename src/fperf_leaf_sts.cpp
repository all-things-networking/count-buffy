#include "fperf_leaf_sts.hpp"

#include <map>
#include <ranges>

#include "Buff.hpp"
#include "prio_sts.hpp"
#include <set>

using namespace views;


void FperfLeafSts::setup() {
    auto src_ports = get_in_ports();
    auto dst_ports = get_out_ports();

    int src_port_count = src_ports.size();
    int dst_port_count = dst_ports.size();

    for (int src_port: src_ports) {
        auto dst_buffs_for_src = get_buffs_for_src(src_port);
        for (int dst_idx = 0; dst_idx < dst_buffs_for_src.size(); ++dst_idx) {
            auto dst_port = dst_buffs_for_src[dst_idx]->dst;
            src_dst_to_dst_idx[src_port][dst_port] = dst_idx;
        }
    }

    for (int dst_port: dst_ports) {
        auto src_buffs_for_dst = get_buffs_for_dst(dst_port);
        for (int src_idx = 0; src_idx < src_buffs_for_dst.size(); src_idx++) {
            auto src_port = src_buffs_for_dst[src_idx]->src;
            dst_src_to_src_idx[dst_port][src_port] = src_idx;
        }
    }

    for (int src_port: src_ports) {
        auto dst_buffs_for_src = get_buffs_for_src(src_port);
        for (int dst_port_idx = 0; dst_port_idx < dst_buffs_for_src.size(); dst_port_idx++) {
            in_to_out_[src_port].push_back(vector<expr>());
            in_prio_head_[src_port].push_back(vector<expr>());
            for (int t = 0; t < timesteps; t++) {
                string vname = format("%s_in_to_out[%d][%d][%d]", var_prefix, src_port, dst_port_idx, t);
                in_to_out_[src_port][dst_port_idx].push_back(slv.ctx.bool_const(vname.c_str()));

                vname = format("%s_in_prio_head[%d][%d][%d]", var_prefix, src_port, dst_port_idx, t);
                in_prio_head_[src_port][dst_port_idx].push_back(slv.ctx.bool_const(vname.c_str()));
            }
        }
    }

    for (int dst_port: dst_ports) {
        auto src_buffs_for_dst = get_buffs_for_dst(dst_port);
        for (int src_port_idx = 0; src_port_idx < src_buffs_for_dst.size(); ++src_port_idx) {
            out_from_in_[dst_port].push_back(vector<expr>());
            out_prio_head_[dst_port].push_back(vector<expr>());
            for (int t = 0; t < timesteps; ++t) {
                string vname = format("%s_out_from_in[%d][%d][%d]", var_prefix, dst_port, src_port_idx, t);
                out_from_in_[dst_port][src_port_idx].push_back(slv.ctx.bool_const(vname.c_str()));

                vname = format("%s_out_prio_head[%d][%d][%d]", var_prefix, dst_port, src_port_idx, t);
                out_prio_head_[dst_port][src_port_idx].push_back(slv.ctx.bool_const(vname.c_str()));
            }
        }
    }
}

FperfLeafSts::FperfLeafSts(SmtSolver &slv, const string &var_prefix, vector<tuple<int, int> > port_list,
                           const int time_steps,
                           const int pkt_types,
                           const int buff_cap,
                           const int max_enq,
                           const int max_deq
) : LeafBase(slv, var_prefix, port_list, time_steps, pkt_types, buff_cap, max_enq, max_deq) {
    setup();
}

FperfLeafSts::FperfLeafSts(SmtSolver &slv, const string &var_prefix, map<tuple<int, int>, vector<int> > port_list,
                           const int time_steps,
                           const int pkt_types,
                           const int buff_cap,
                           const int max_enq,
                           const int max_deq
) : LeafBase(slv, var_prefix, port_list, time_steps, pkt_types, buff_cap, max_enq, max_deq) {
    setup();
}

vector<NamedExp> FperfLeafSts::out(int t) {
    vector<NamedExp> constrs;
    auto src_ports = get_in_ports();
    for (auto src_port: src_ports) {
        auto dst_buffs_for_src = get_buffs_for_src(src_port);
        for (int i = 0; i < dst_buffs_for_src.size(); ++i) {
            auto buff = dst_buffs_for_src[i];
            auto constr_name = format("%s_in_%d_deq_cnt[%d]_is_one", var_prefix, i, t);
            expr constr_expr = implies(in_to_out_[src_port][i][t], buff->O[t] == 1);
            constrs.emplace_back(constr_expr, constr_name);

            constr_name = format("%s_in_%d_deq_cnt[%d]_is_zero", var_prefix, i, t);
            constr_expr = implies(!in_to_out_[src_port][i][t], buff->O[t] == 0);
            constrs.emplace_back(constr_expr, constr_name);
        }
    }

    auto dst_ports = get_out_ports();
    for (auto dst_port: dst_ports) {
        auto src_buffs_for_dst = get_buffs_for_dst(dst_port);

        expr_vector match_all_zeros(slv.ctx);
        match_all_zeros.push_back(slv.ctx.bool_val(true));

        expr_vector out_all_zeros(slv.ctx);
        out_all_zeros.push_back(slv.ctx.bool_val(true));

        for (auto i = 0; i < src_buffs_for_dst.size(); i++) {
            match_all_zeros.push_back(!out_from_in_[dst_port][i][t]);
            out_all_zeros.push_back(src_buffs_for_dst[i]->O[t] == 0);
        }
        string constr_name = format("%s_no_enqs_in_out_%d_at_%d", var_prefix, dst_port, t);
        expr constr_expr = implies(mk_and(match_all_zeros), mk_and(out_all_zeros));
        constrs.emplace_back(constr_expr, constr_name);
    }
    return constrs;
}

vector<NamedExp> FperfLeafSts::trs(int prev_t) {
    vector<NamedExp> constrs;

    int t = prev_t + 1;
    auto src_ports = get_in_ports();
    auto dst_ports = get_out_ports();

    //  input and output match together
    for (const auto &src_dst: buffs | keys) {
        int src_port = get<0>(src_dst);
        int dst_port = get<1>(src_dst);
        int src_port_idx = dst_src_to_src_idx[dst_port][src_port];
        int dst_port_idx = src_dst_to_dst_idx[src_port][dst_port];

        string constr_name = format(
            "%s_in_and_out_together_%d(q%d)_%d(q%d)_at_%d",
            var_prefix,
            src_port,
            dst_port_idx,
            dst_port,
            src_port_idx,
            t);
        expr constr_expr = (implies(in_to_out_[src_port][dst_port_idx][t],
                                    out_from_in_[dst_port][src_port_idx][t]) &&
                            implies(!in_to_out_[src_port][dst_port_idx][t],
                                    !out_from_in_[dst_port][src_port_idx][t]));
        constrs.emplace_back(constr_expr, constr_name);
    }

    for (const auto &[src_dst,buff]: buffs) {
        int src_port = get<0>(src_dst);
        int dst_port = get<1>(src_dst);
        int src_idx_in_dst = dst_src_to_src_idx[dst_port][src_port];
        int dst_idx_in_src = src_dst_to_dst_idx[src_port][dst_port];
        auto dst_buffs_for_src = get_buffs_for_src(src_port);
        auto src_buffs_for_dst = get_buffs_for_dst(dst_port);

        expr_vector cond1(slv.ctx);
        int num_src_buffers_for_dst = src_buffs_for_dst.size();
        for (int p_src_idx_in_dst = 0; p_src_idx_in_dst < num_src_buffers_for_dst; p_src_idx_in_dst++) {
            expr_vector not_matched(slv.ctx);
            not_matched.push_back(slv.ctx.bool_val(true));
            for (int k_src_idx_in_dst = 0; k_src_idx_in_dst < num_src_buffers_for_dst; k_src_idx_in_dst++) {
                int ind = (p_src_idx_in_dst + 1 + k_src_idx_in_dst) % num_src_buffers_for_dst;
                if (ind == src_idx_in_dst) break;
                not_matched.push_back(!out_from_in_[dst_port][ind][t]);
            }
            cond1.push_back(implies(out_prio_head_[dst_port][p_src_idx_in_dst][t], mk_and(not_matched)));
        }

        // no higher priority outputs
        expr_vector cond2(slv.ctx);
        int num_dst_buffers_for_src = dst_buffs_for_src.size();
        for (int p_dst_idx_in_src = 0; p_dst_idx_in_src < num_dst_buffers_for_src; p_dst_idx_in_src++) {
            expr_vector not_matched(slv.ctx);
            not_matched.push_back(slv.ctx.bool_val(true));
            for (unsigned int k_dst_idx_in_src = 0; k_dst_idx_in_src < num_dst_buffers_for_src; k_dst_idx_in_src++) {
                unsigned int ind = (p_dst_idx_in_src + 1 + k_dst_idx_in_src) % num_dst_buffers_for_src;
                if (ind == dst_idx_in_src) break;
                not_matched.push_back(!in_to_out_[src_port][ind][t]);
            }
            cond2.push_back(implies(in_prio_head_[src_port][p_dst_idx_in_src][t], mk_and(not_matched)));
        }

        expr cond3 = buff->B[t];

        expr match_cond = mk_and(cond1) && mk_and(cond2) && cond3;

        string constr_name = format("%s_%d_matches_%d_at_%d", var_prefix, src_port, dst_port, t);
        expr constr_expr = implies(match_cond, in_to_out_[src_port][dst_idx_in_src][t]);
        constrs.emplace_back(constr_expr, constr_name);

        constr_name = format("%s_%d_doesnt_match_%d_at_%d", var_prefix, src_port, dst_port, t);
        constr_expr = implies(!match_cond, !in_to_out_[src_port][dst_idx_in_src][t]);
        constrs.emplace_back(constr_expr, constr_name);
    }


    // No double matching in input
    for (int src_port: dst_ports) {
        auto dst_buffs_for_src = get_buffs_for_src(src_port);
        for (int i = 0; i < dst_buffs_for_src.size(); ++i) {
            expr_vector others_zero(slv.ctx);
            for (int j = 0; j < dst_buffs_for_src.size(); ++j) {
                if (i == j)
                    continue;
                others_zero.push_back(in_to_out_[src_port][j][t]);
            }
            string constr_name = format(
                "%s_in_port_%d_matches_only_voq_%d_at_%d",
                var_prefix,
                src_port,
                i,
                t);
            expr constr_expr = implies(in_to_out_[src_port][i][t], mk_and(others_zero));
            constrs.emplace_back(constr_expr, constr_name);
        }
    }

    // No double matching in input
    for (int dst_port: dst_ports) {
        auto src_buffs_for_dst = get_buffs_for_dst(dst_port);
        for (int i = 0; i < src_buffs_for_dst.size(); ++i) {
            expr_vector others_zero(slv.ctx);
            for (int j = 0; j < src_buffs_for_dst.size(); ++j) {
                if (i == j)
                    continue;
                others_zero.push_back(out_from_in_[dst_port][j][t]);
            }

            string constr_name = format(
                "%s_in_port_%d_matches_only_voq_%d_at_%d",
                var_prefix,
                dst_port,
                i,
                t);
            expr constr_expr = implies(out_from_in_[dst_port][i][t], mk_and(others_zero));
            constrs.emplace_back(constr_expr, constr_name);
        }
    }


    // cout << 5 << endl;
    //  One of prios is always one, and only one is one
    for (auto src_port: dst_ports) {
        expr_vector all_prios(slv.ctx);
        auto dst_buffs_for_src = get_buffs_for_src(src_port);
        for (int i = 0; i < dst_buffs_for_src.size(); ++i) {
            all_prios.push_back(in_prio_head_[src_port][i][t]);
        }

        string constr_name = format("%s_at_least_one_in_prio_head_[%d]_at_%d", var_prefix, src_port, t);
        expr constr_expr = mk_or(all_prios);
        constrs.emplace_back(constr_expr, constr_name);

        for (int i = 0; i < dst_buffs_for_src.size(); ++i) {
            expr_vector others_zero(slv.ctx);
            for (int j = 0; j < dst_buffs_for_src.size(); ++j) {
                if (i == j) continue;
                others_zero.push_back(!in_prio_head_[src_port][j][t]);
            }
            constr_name = format("%s_only_in_prio_head[%d][%d][%d]_is_one", var_prefix, src_port, i, t);
            constr_expr = implies(in_prio_head_[src_port][i][t], mk_and(others_zero));
            constrs.emplace_back(constr_expr, constr_name);
        }
    }

    //  One of prios is always one, and only one is one
    for (auto dst_port: dst_ports) {
        expr_vector all_prios(slv.ctx);
        auto src_buffs_for_dst = get_buffs_for_dst(dst_port);
        for (int i = 0; i < src_buffs_for_dst.size(); ++i) {
            all_prios.push_back(out_prio_head_[dst_port][i][t]);
        }

        string constr_name = format("%s_at_least_one_out_prio_head_[%d]_at_%d", var_prefix, dst_port, t);
        expr constr_expr = mk_or(all_prios);
        constrs.emplace_back(constr_expr, constr_name);

        for (int i = 0; i < src_buffs_for_dst.size(); ++i) {
            expr_vector others_zero(slv.ctx);
            for (int j = 0; j < src_buffs_for_dst.size(); ++j) {
                if (i == j) continue;
                others_zero.push_back(!out_prio_head_[dst_port][j][t]);
            }
            constr_name = format("%s_only_out_prio_head[%d][%d][%d]_is_one", var_prefix, dst_port, i, t);
            constr_expr = implies(out_prio_head_[dst_port][i][t], mk_and(others_zero));
            constrs.emplace_back(constr_expr, constr_name);
        }
    }

    if (t > 0) {
        for (auto src_port: src_ports) {
            auto dst_buffs_for_src = get_buffs_for_src(src_port);
            for (int i = 0; i < dst_buffs_for_src.size(); ++i) {
                string constr_name = format(
                    "%s_update_in_prio_head[%d][%d][%d]_to_one",
                    var_prefix,
                    src_port,
                    i,
                    t);
                expr constr_expr = implies(in_to_out_[src_port][i][prev_t], in_prio_head_[src_port][i][t]);
                constrs.emplace_back(constr_expr, constr_name);
            }

            expr_vector all_zero(slv.ctx);
            for (int i = 0; i < dst_buffs_for_src.size(); ++i) {
                all_zero.push_back(!in_to_out_[src_port][i][prev_t]);
            }

            for (unsigned int i = 0; i < dst_buffs_for_src.size(); i++) {
                string constr_name =
                        format("%s_dont_update_in_prio_head[%d][%d][%d]",
                               var_prefix,
                               src_port,
                               i,
                               t);
                expr constr_expr = implies(mk_and(all_zero),
                                           in_prio_head_[src_port][i][t] ==
                                           in_prio_head_[src_port][i][prev_t]);
                constrs.emplace_back(constr_expr, constr_name);
            }
        }

        for (auto dst_port: dst_ports) {
            auto src_buffs_for_dst = get_buffs_for_dst(dst_port);
            for (int i = 0; i < src_buffs_for_dst.size(); ++i) {
                string constr_name = format(
                    "%s_update_out_prio_head[%d][%d][%d]_to_one",
                    var_prefix,
                    dst_port,
                    i,
                    t);
                expr constr_expr = implies(out_prio_head_[dst_port][i][prev_t], out_from_in_[dst_port][i][t]);
                constrs.emplace_back(constr_expr, constr_name);
            }

            expr_vector all_zero(slv.ctx);
            for (int i = 0; i < src_buffs_for_dst.size(); ++i) {
                all_zero.push_back(!out_prio_head_[dst_port][i][prev_t]);
            }

            for (unsigned int i = 0; i < src_buffs_for_dst.size(); i++) {
                string constr_name =
                        format("%s_dont_update_out_prio_head[%d][%d][%d]",
                               var_prefix,
                               dst_port,
                               i,
                               t);
                expr constr_expr = implies(mk_and(all_zero),
                                           out_prio_head_[dst_port][i][t] ==
                                           out_prio_head_[dst_port][i][prev_t]);
                constrs.emplace_back(constr_expr, constr_name);
            }
        }
    }

    return constrs;
}

vector<NamedExp> FperfLeafSts::init() {
    vector<NamedExp> constrs;
    trs(-1);
    auto src_ports = get_in_ports();
    auto dst_ports = get_out_ports();

    for (auto src_port: dst_ports) {
        auto dst_buffs_for_src = get_buffs_for_src(src_port);
        if (dst_buffs_for_src.size() > 0) {
            string constr_name = format("%s_in_prio_head[%d][0][0]_is_one", var_prefix, src_port);
            expr constr_expr = in_prio_head_[src_port][0][0];

            for (unsigned int i = 1; i < dst_buffs_for_src.size(); i++) {
                constr_name = format("%s_in_prio_head[%d][%d][0]_is_zero", var_prefix, src_port, i);
                constr_expr = !in_prio_head_[src_port][i][0];
                constrs.emplace_back(constr_expr, constr_name);
            }
        }
    }

    for (auto dst_port: dst_ports) {
        auto src_buffs_for_dst = get_buffs_for_dst(dst_port);
        if (src_buffs_for_dst.size() > 0) {
            string constr_name = format("%s_out_prio_head[%d][0][0]_is_one", var_prefix, dst_port);
            expr constr_expr = out_prio_head_[dst_port][0][0];

            for (unsigned int i = 1; i < src_buffs_for_dst.size(); i++) {
                constr_name = format("%s_out_prio_head[%d][%d][0]_is_zero", var_prefix, dst_port, i);
                constr_expr = !out_prio_head_[dst_port][i][0];
                constrs.emplace_back(constr_expr, constr_name);
            }
        }
    }
    return constrs;
}
