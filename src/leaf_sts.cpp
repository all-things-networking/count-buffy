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
): LeafBase(slv, var_prefix, port_list, time_steps, pkt_types, buff_cap, max_enq, max_deq) {
}

LeafSts::LeafSts(SmtSolver &slv, const string &var_prefix, map<tuple<int, int>, vector<int> > port_list,
                 const int time_steps,
                 const int pkt_types,
                 const int buff_cap,
                 const int max_enq,
                 const int max_deq
): LeafBase(slv, var_prefix, port_list, time_steps, pkt_types, buff_cap, max_enq, max_deq) {
}
