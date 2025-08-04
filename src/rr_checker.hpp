//
// Created by Amir Hossein Seyhani on 3/17/25.
//

#ifndef RR_CHECKER_HPP
#define RR_CHECKER_HPP

#include "sts_checker.hpp"


class RRChecker final : public STSChecker {
public:
    RRChecker(SmtSolver &slv, const string& var_prefix, const int n, const int m, const int k, const int c,
              const int me, const int md)
        : STSChecker(slv, var_prefix, n, m, k, c, me, md) {
    }

    vector<NamedExp> workload() override;

    vector<NamedExp> base_wl();

    vector<NamedExp> out(const ev &bv, const ev &sv, const ev2 &ov, int t) override;

    vector<NamedExp> trs(const ev &b, const ev &s, const ev &bp, const ev &sp, int tp) override;

    vector<NamedExp> query(int m) override;

    vector<NamedExp> init(ev const &b0, ev const &s0) override;
};


#endif //RR_CHECKER_HPP
