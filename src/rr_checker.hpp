//
// Created by Amir Hossein Seyhani on 3/17/25.
//

#ifndef RR_CHECKER_HPP
#define RR_CHECKER_HPP

#include "sts_checker.hpp"


class RRChecker final : public STSChecker {
public:
    RRChecker(SmtSolver &slv, const string &var_prefix, const int n, const int m, const int k, const int c,
              const int me, const int md)
        : STSChecker(slv, var_prefix, n, m, k, c, me, md) {
    }

    expr workload() override;

    expr out(const ev &bv, const ev &sv, const ev2 &ov) override;

    expr trs(const ev &b, const ev &s, const ev &bp, const ev &sp) override;

    expr query(int m) override;

    expr init(ev const &b0, ev const &s0) override;
};


#endif //RR_CHECKER_HPP
