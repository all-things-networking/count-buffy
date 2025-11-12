//
// Created by Amir Hossein Seyhani on 10/8/25.
//

#ifndef LEAF_WORKLOAD_PARSER_HPP
#define LEAF_WORKLOAD_PARSER_HPP
#include <map>
#include <set>

#include "wl_parser.hpp"


class LeafWorkloadParser {
public:
    SmtSolver &slv;
    int timesteps;
    vector<int> max_t_with_zero_cenq;
    map<int, vector<int> > dst_to_pkt_type;
    map<int, vector<int> > ecmp_to_pkt_type;
    vector<int> all_pkt_types;
    vector<tuple<int, int, string, int> > dst_constrs;
    vector<tuple<int, int, string, int> > ecmp_constrs;
    ev3 I;

    void merge(vector<int> max_t_update);

    set<int> get_zero_inputs();

    LeafWorkloadParser(SmtSolver &slv, ev3 &I, int timesteps, map<int, int> pkt_type_to_dst,
                       map<int, int> pkt_type_to_ecmp);

    vector<NamedExp> parse(string prefix, string wl_line);

    void parse(vector<string> wl);

    expr base_wl();

    expr query();
};


#endif //LEAF_WORKLOAD_PARSER_HPP
