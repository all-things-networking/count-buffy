//
// Created by Amir Hossein Seyhani on 3/17/25.
//

#ifndef PRIO_STS_HPP
#define PRIO_STS_HPP
#include "sts_checker.hpp"

class PrioSTS final : public STSChecker {
public:
    PrioSTS(int n, int k, int c, int me, int md);

    expr workload(int n) override;

    expr out(const ev &bv, const ev &ov) override;

    expr trs(const evv &B, int n) override;

    expr query(int m) override;
};


#endif //PRIO_STS_HPP
