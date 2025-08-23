//
// Created by Amir Hossein Seyhani on 3/17/25.
//

#ifndef PRIO_STS_HPP
#define PRIO_STS_HPP
#include "sts_checker.hpp"

class LeafSts final : public STSChecker {
public:
    LeafSts(SmtSolver &slv, const string &var_prefix, int n, int m, int k, int c, int me, int md);

    LeafSts(SmtSolver &slv, const string &var_prefix, int n, int num_ports, int c, int me, int md);

    vector<NamedExp> workload() override;

    vector<NamedExp> out(const ev &bv, const ev &sv, const ev2 &ov, int t) override;

    expr rr(ev const &backlog, expr &prev_turn);

    vector<NamedExp> trs(ev const &b, ev const &s, ev const &bp, ev const &sp, int tp) override;

    vector<NamedExp> query(int p) override;

    vector<NamedExp> init(const ev &b0, const ev &s0) override;

    template<class V>
    V get_voq_of_out_i(const V &all_ev, int i);

    void print(model m) const override;

    int num_ports;
};


#endif //PRIO_STS_HPP
