//
// Created by Amir Hossein Seyhani on 10/8/25.
//

#ifndef LEAF_WORKLOAD_PARSER_HPP
#define LEAF_WORKLOAD_PARSER_HPP
#include <map>

#include "wl_parser.hpp"


class LeafWorkloadParser {
public:
    SmtSolver &slv;
    int timesteps;
    int num_buffs;
    map<int, vector<int> > dst_to_pkt_type;
    map<int, vector<int> > ecmp_to_pkt_type;
    vector<int> all_pkt_types;
    ev3 I;

    LeafWorkloadParser(SmtSolver &slv, ev3 &I, int num_spines, int num_leafs, int host_per_leaf, int timesteps);

    expr parse(string prefix, string wl_line);

    void parse(vector<string> wl);
};


#endif //LEAF_WORKLOAD_PARSER_HPP
