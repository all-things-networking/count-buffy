//
// Created by Amir Hossein Seyhani on 4/22/25.
//
#ifndef TRIVIAL_HPP
#define TRIVIAL_HPP

#include "sts_checker.hpp"

class TrivialSts : public STSChecker {
public:
    TrivialSts(SmtSolver &slv, const string &var_prefix, int n, int m, int k, int c, int me, int md): STSChecker(
        slv, var_prefix, n, m, k, c, me, md) {
    }

    expr workload() override;

    expr out(const ev &bv, const ev &sv, const ev2 &ov) override;

    expr trs(const ev &b, const ev &s, const ev &bp, const ev &sp) override;

    expr init(const ev &b0, const ev &s0) override;

    expr query(int m) override;
};

#endif //TRIVIAL_HPP
