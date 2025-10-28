//
// Created by Amir Hossein Seyhani on 3/17/25.
//

#ifndef PRIO_STS_HPP
#define PRIO_STS_HPP

#include "leaf_base.hpp"

class LeafSts : public LeafBase {
public:
    LeafSts(SmtSolver &slv,
            const string &var_prefix,
            vector<tuple<int, int> > port_list,
            int time_steps,
            int pkt_types,
            int buf_cap,
            int max_enq,
            int max_deq);

    LeafSts(SmtSolver &slv,
            const string &var_prefix,
            map<tuple<int, int>, vector<int> > port_list,
            int time_steps,
            int pkt_types,
            int buf_cap,
            int max_enq,
            int max_deq);
};


#endif //PRIO_STS_HPP
