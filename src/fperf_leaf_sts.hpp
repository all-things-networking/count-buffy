//
// Created by Amir Hossein Seyhani on 3/17/25.
//

#ifndef FPERF_PRIO_STS_HPP
#define FPERF_PRIO_STS_HPP

#include "leaf_base.hpp"

class FperfLeafSts : public LeafBase {
public:
    FperfLeafSts(SmtSolver &slv,
                 const string &var_prefix,
                 vector<tuple<int, int> > port_list,
                 int time_steps,
                 int pkt_types,
                 int buf_cap,
                 int max_enq,
                 int max_deq);

    FperfLeafSts(SmtSolver &slv,
                 const string &var_prefix,
                 map<tuple<int, int>, vector<int> > port_list,
                 int time_steps,
                 int pkt_types,
                 int buf_cap,
                 int max_enq,
                 int max_deq);

    void setup();

    vector<NamedExp> out(int t) override;

    vector<NamedExp> trs(int t) override;

    vector<NamedExp> init() override;

    map<int, map<int, int> > dst_src_to_src_idx;
    map<int, map<int, int> > src_dst_to_dst_idx;

    map<int, vector<vector<expr>>> in_to_out_;
    map<int, vector<vector<expr>>> out_from_in_;

    map<int, vector<vector<expr>>> in_prio_head_;
    map<int, vector<vector<expr>>> out_prio_head_;
};


#endif
