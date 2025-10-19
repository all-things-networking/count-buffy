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
    map<int, vector<int> > dst_to_pkt_type;
    map<int, vector<int> > ecmp_to_pkt_type;
    vector<int> all_pkt_types;
    ev3 I;

    LeafWorkloadParser(SmtSolver &slv, ev3 &I, int timesteps, map<int, int> pkt_type_to_dst,
                       map<int, int> pkt_type_to_ecmp);

    vector<NamedExp> parse(string prefix, string wl_line);

    void parse(vector<string> wl);

    expr base_wl();

    expr query();
};


#endif //LEAF_WORKLOAD_PARSER_HPP
