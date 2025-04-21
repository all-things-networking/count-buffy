#include <iostream>
#include <set>

#include"z3++.h"
#include "src/rr_checker.hpp"
#include "src/merger.hpp"

using namespace std;
using namespace z3;

constexpr int MAX_ENQ = 4;
constexpr int MAX_DEQ = 1;

const int N = 10;
const int RR_IN_BUFS = 2;
const int PKT_TYPES = 2;
const int C = 10;

expr query(SmtSolver &slv, ev3 &out) {
    expr res = slv.ctx.bool_val(true);
    ev2 ov = out[0];
    ev s = ov[0];
    for (int i = 1; i < ov.size(); ++i) {
        s = s + ov[i];
    }
    res = res & ((s[1] - s[0]) >= 3);
    return res;
}

expr wl(SmtSolver &slv, ev4 &ins) {
    expr res = slv.ctx.bool_val(true);

    int num_rrs = ins.size();
    int num_in_bufs = ins[0].size();
    int timesteps = ins[0][0].size();

    for (int k = 0; k < num_in_bufs; ++k) {
        for (int i = 0; i < timesteps; ++i) {
            auto s = ins[0][0][i];
            for (int j = 1; j < num_rrs; ++j) {
                s = s + ins[j][0][i];
            }
            for (int kp = 0; kp < num_in_bufs; ++kp) {
                if (k == kp)
                    res = res & (s[kp] >= i);
                else
                    res = res & (s[kp] == 0);
            }
        }
    }

    for (int i = 0; i < ins.size(); ++i) {
    }
}

expr base_wl(SmtSolver &slv, ev3 &v3, set<int> ids) {
}

expr base_wl(SmtSolver &slv, ev4 &ins) {
    expr res = slv.ctx.bool_val(true);

    int num_rrs = ins.size();
    int num_in_bufs = ins[0].size();
    int timesteps = ins[0][0].size();

    for (int k = 0; k < num_in_bufs; ++k) {
        for (int i = 0; i < timesteps; ++i) {
            auto s = ins[0][0][i];
            for (int j = 1; j < num_rrs; ++j) {
                s = s + ins[j][0][i];
            }
            for (int kp = 0; kp < num_in_bufs; ++kp) {
                if (k == kp)
                    res = res & (s[kp] >= i);
                else
                    res = res & (s[kp] == 0);
            }
        }
    }

    for (int i = 0; i < ins.size(); ++i) {
    }
}

int main(const int argc, const char *argv[]) {
    SmtSolver slv;
    STSChecker *rr1 = new RRChecker(slv, "rr1", N, RR_IN_BUFS, PKT_TYPES, C, MAX_ENQ, MAX_DEQ);
    STSChecker *rr2 = new RRChecker(slv, "rr2", N, RR_IN_BUFS, PKT_TYPES, C, MAX_ENQ, MAX_DEQ);
    STSChecker *rr3 = new RRChecker(slv, "rr3", N, RR_IN_BUFS, PKT_TYPES, C, MAX_ENQ, MAX_DEQ);
    STSChecker *rr4 = new RRChecker(slv, "rr4", N, RR_IN_BUFS, PKT_TYPES, C, MAX_ENQ, MAX_DEQ);
    // STSChecker *merger = new Merger(slv, "mg", N, RR_IN_BUFS, PKT_TYPES, C, MAX_ENQ, MAX_DEQ);;
}
