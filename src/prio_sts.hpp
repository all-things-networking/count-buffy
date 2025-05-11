//
// Created by Amir Hossein Seyhani on 3/17/25.
//

#ifndef PRIO_STS_HPP
#define PRIO_STS_HPP
#include "sts_checker.hpp"

class PrioSTS final : public STSChecker {
public:
    PrioSTS(SmtSolver &slv, const string &var_prefix, int n, int m, int k, int c, int me, int md);

    vector<NamedExp> workload() override;

    vector<NamedExp> out(const ev &bv, const ev &sv, const ev2 &ov) override;

    vector<NamedExp> trs(ev const &b, ev const &s, ev const &bp, ev const &sp) override;

    vector<NamedExp> query(int p) override;

    vector<NamedExp> init(const ev &b0, const ev &s0) override;
};


#endif //PRIO_STS_HPP
