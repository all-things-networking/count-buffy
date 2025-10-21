//
// Created by Amir Hossein Seyhani on 8/25/25.
//

#ifndef DEMUX_SWITCH_HPP
#define DEMUX_SWITCH_HPP
#include "leaf_sts.hpp"


class DemuxSwitch : public LeafSts {
public:
    void add_some_constraint(SmtSolver &slv, const string &var_prefix, int pkt_types, vector<int> pkt_type_to_nxt_hop);

    DemuxSwitch(SmtSolver &slv,
                const string &var_prefix,
                map<tuple<int, int>, vector<int> > port_list,
                int time_steps,
                int pkt_types,
                int buf_cap,
                int max_enq,
                int max_deq,
                vector<int> pkt_type_to_nxt_hop
    );

    DemuxSwitch(SmtSolver &slv,
                const string &var_prefix,
                vector<tuple<int, int> > port_list,
                int time_steps,
                int pkt_types,
                int buf_cap,
                int max_enq,
                int max_deq,
                vector<int> pkt_type_to_nxt_hop
    );

    map<int, Buff *> get_dst_map(int src);

    ev2 get_in_port(int src) override;

    vector<int> pkt_type_to_nxt_hop; //DST port for each pkt type
};


#endif //DEMUX_SWITCH_HPP
