//
// Created by Amir Hossein Seyhani on 7/28/25.
//

#ifndef FQ_CHECKER_HPP
#define FQ_CHECKER_HPP
#include "IntSeq.hpp"
#include "sts_checker.hpp"


class FqChecker : public STSChecker {
public:
    FqChecker(SmtSolver &slv, const string &var_prefix, int n, int m, int k, int c, int me, int md);

    ev2 SS;
    ev2 nq;
    ev2 oq;
    ev2 tmp;
    ev2 ipn;
    ev2 ipo;

    vector<NamedExp> out(const ev &bv, const ev &sv, const ev2 &ov, int t) override;

    vector<NamedExp> trs(const ev &b, const ev &s, const ev &bp, const ev &sp, int tp) override;

    vector<NamedExp> init(const ev &b0, const ev &s0) override;

    vector<NamedExp> workload() override;

    vector<NamedExp> query(int m) override;

    ev2 get_state() const override;

    vector<IntSeq> nqs;

    vector<IntSeq> oqs;
};


#endif //FQ_CHECKER_HPP
