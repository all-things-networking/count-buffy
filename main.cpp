#include <iostream>
#include<vector>
#include"z3++.h"
#include "src/prio_sts.hpp"
#include "src/rr_checker.hpp"

using namespace std;
using namespace z3;

typedef vector<expr> ev;
typedef vector<ev> evv;

constexpr int MAX_ENQ = 4;
constexpr int MAX_DEQ = 1;

constexpr int INP_CALC = 3;


int main(const int argc, const char *argv[]) {
    if (argc < 4)
        return 1;
    int n = atoi(argv[1]);
    int k = atoi(argv[2]);
    int c = atoi(argv[3]);
    // STSChecker *sts = new PrioSTS(n, k, c, MAX_ENQ, MAX_DEQ);
    STSChecker *sts = new RRChecker(n, k, c, MAX_ENQ, MAX_DEQ);
    auto m = sts->check_wl_sat();
    sts->print(m);
    sts->check_wl_not_qry_unsat();
    return 0;
}
