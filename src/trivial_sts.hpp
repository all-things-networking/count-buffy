//
// Created by Amir Hossein Seyhani on 5/5/25.
//

#ifndef TRIVIAL_STS_HPP
#define TRIVIAL_STS_HPP
#include "sts_checker.hpp"


class TrivialSts final : public STSChecker {
public:
    TrivialSts(SmtSolver &slv, const string &var_prefix, const int m)
        : STSChecker(slv, var_prefix, 1, m, 2, 4, 4, 4) {
    }

    vector<NamedExp> workload() override;

    vector<NamedExp> out(const ev &bv, const ev &sv, const ev2 &ov, int t) override;

    vector<NamedExp> trs(const ev &b, const ev &s, const ev &bp, const ev &sp, int tp) override;

    vector<NamedExp> query(int m) override;

    vector<NamedExp> init(ev const &b0, ev const &s0) override;
};


#endif //TRIVIAL_STS_HPP
