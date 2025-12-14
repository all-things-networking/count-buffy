//
// Created by Amir Hossein Seyhani on 3/17/25.
//

#ifndef PRIO_STS_HPP
#define PRIO_STS_HPP
#include "../src/sts_checker.hpp"

class RateLimiter final : public STSChecker {
public:
    RateLimiter(SmtSolver &slv, const string &var_prefix, int n, int m, int k, int c, int me, int md);

    vector<NamedExp> workload() override;

    vector<NamedExp> out(const ev &bv, const ev &sv, const ev2 &ov, int t) override;

    vector<NamedExp> trs(ev const &b, ev const &s, ev const &bp, ev const &sp, int tp) override;

    vector<NamedExp> query() override;

    vector<NamedExp> init(const ev &b0, const ev &s0) override;

    void print(model m) const override;
};


#endif //PRIO_STS_HPP
