#include <iostream>
#include<vector>
#include"z3++.h"
#include "src/prio_sts.hpp"
#include "src/rr_checker.hpp"

using namespace std;
using namespace z3;

constexpr int MAX_ENQ = 4;
constexpr int MAX_DEQ = 1;

constexpr int INP_CALC = 3;


int main(const int argc, const char *argv[]) {
    if (argc < 5)
        return 1;
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    int k = atoi(argv[3]);
    int c = atoi(argv[4]);
    string name = argv[5];
    STSChecker *sts;
    if (name == "prio") {
        sts = new PrioSTS("prio", n, m, k, c, MAX_ENQ, MAX_DEQ);
        cout << "PRIO" << endl;
    } else if (name == "rr") {
        sts = new RRChecker("rr", n, m, k, c, MAX_ENQ, MAX_DEQ);
        cout << "RR" << endl;
    }
    auto mod = sts->check_wl_sat();
    // sts->print(mod);
    sts->check_wl_not_qry_unsat();
    return 0;
}
