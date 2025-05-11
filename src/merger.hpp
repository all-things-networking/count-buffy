#ifndef MERGER_HPP
#define MERGER_HPP
#include "sts_checker.hpp"

class Merger : public STSChecker {
public:
    Merger(SmtSolver &slv, const string &var_prefix, const int n, const int m, const int k, const int c,
           const int me, const int md)
        : STSChecker(slv, var_prefix, n, m, k, c, me, md) {
    }

    vector<NamedExp> out(const ev &bv, const ev &sv, const ev2 &ov) override;

    vector<NamedExp> trs(ev const &b, ev const &s, ev const &bp, ev const &sp) override;

    vector<NamedExp> init(ev const &b0, ev const &s0) override;
};


#endif
