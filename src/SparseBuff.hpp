//
// Created by Amir Hossein Seyhani on 10/30/25.
//

#ifndef SPARSEBUFF_HPP
#define SPARSEBUFF_HPP
#include "Buff.hpp"
#include "lib.hpp"


class SparseBuff : public Buff {
public:
    SparseBuff(SmtSolver &slv, const string &var_prefix, int time_steps, int pkt_types, int max_enq, int max_deq,
               int buf_cap,
               int src, int dst, vector<int> used_pkt_types);

    ev2 getI() const override;
    expr meta_ecmp;
    expr meta_dst;
private:
    ev2 I;
};


#endif //SPARSEBUFF_HPP
