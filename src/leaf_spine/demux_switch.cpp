//
// Created by Amir Hossein Seyhani on 8/25/25.
//

#include "demux_switch.hpp"

void DemuxSwitch::add_some_constraint(SmtSolver &slv, const string &var_prefix, int pkt_types,
                                      vector<int> pkt_type_to_nxt_hop) {
    for (int src: get_in_ports()) {
        auto dst_buffs = get_dst_map(src);
        for (const auto &[dst, buff]: dst_buffs) {
            for (int t = 0; t < timesteps; ++t) {
                for (int k = 0; k < pkt_types; ++k) {
                    int nxt_hop_dst_port = pkt_type_to_nxt_hop[k];
                    if (dst != nxt_hop_dst_port) {
                        string constr_name = format("{}_[{}->{}@{}#{}] == 0", var_prefix, src, dst, t, k);
                        slv.add(buff->getI()[t][k] == 0, constr_name);
                    }
                }
            }
        }
    }
}

DemuxSwitch::DemuxSwitch(SmtSolver &slv, const string &var_prefix, map<tuple<int, int>, vector<int> > port_list,
                         int time_steps,
                         int pkt_types, int buf_cap, int max_enq, int max_deq,
                         vector<int> pkt_type_to_nxt_hop) : FperfLeafSts(
                                                                slv, var_prefix, port_list, time_steps, pkt_types,
                                                                buf_cap, max_enq, max_deq),
                                                            pkt_type_to_nxt_hop(pkt_type_to_nxt_hop) {
    // add_some_constraint(slv, var_prefix, pkt_types, pkt_type_to_nxt_hop);
}

DemuxSwitch::DemuxSwitch(SmtSolver &slv, const string &var_prefix, vector<tuple<int, int> > port_list, int time_steps,
                         int pkt_types, int buf_cap, int max_enq, int max_deq,
                         vector<int> pkt_type_to_nxt_hop) : FperfLeafSts(
                                                                slv, var_prefix, port_list, time_steps, pkt_types,
                                                                buf_cap, max_enq, max_deq),
                                                            pkt_type_to_nxt_hop(pkt_type_to_nxt_hop) {
    // add_some_constraint(slv, var_prefix, pkt_types, pkt_type_to_nxt_hop);
}

map<int, LeafBuff *> DemuxSwitch::get_dst_map(int src) {
    map<int, LeafBuff *> result;
    for (const auto &[key, buff]: buffs) {
        int s = get<0>(key);
        int d = get<1>(key);
        if (src == s)
            result[d] = buff;
    }
    return result;
}

ev2 DemuxSwitch::get_in_port(int src) {
    // vector<Buff *> src_buffs = get_buffs_for_src(src);
    auto dst_buffs = get_dst_map(src);
    // assert(src_buffs.size() > 0);
    ev2 in;

    for (int t = 0; t < timesteps; ++t) {
        ev vals;
        for (int k = 0; k < pkt_types; ++k) {
            expr val = slv.ctx.int_val(0);

            int nxt_hop_dst_port = pkt_type_to_nxt_hop[k];
            if (dst_buffs.contains(nxt_hop_dst_port)) {
                LeafBuff *nxt_hop_buff = dst_buffs[nxt_hop_dst_port];
                auto I = nxt_hop_buff->getExpandedI();
                val = I[t][k];
            }
            vals.push_back(val);
        }
        assert(vals.size() == pkt_types);
        in.push_back(vals);
    }
    assert(in.size() == timesteps);
    return in;
}
