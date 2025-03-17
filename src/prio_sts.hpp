//
// Created by Amir Hossein Seyhani on 3/17/25.
//

#ifndef PRIO_STS_HPP
#define PRIO_STS_HPP
#include "sts_checker.hpp"

class PrioSTS final : public STSChecker {
public:
    PrioSTS(int n, int k, int c, int me, int md);

    expr workload() override;

    expr out(const ev &bv, const ev &sv, const ev &ov) override;

    expr trs(ev const &b, ev const &s, ev const &bp, ev const &sp) override;

    expr query(int m) override;

    expr init(const ev &b0, const ev &s0) override;
};


#endif //PRIO_STS_HPP
