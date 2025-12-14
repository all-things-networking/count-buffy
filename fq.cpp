#include <iostream>

#include "antlr4-runtime.h"
#include"z3++.h"
#include "src/FqChecker.hpp"
#include "src/smt_solver.hpp"
#include "src/sts_runner.hpp"
#include "src/utils.hpp"
#include "src/gen/wl_parser.hpp"

using namespace std;
using namespace z3;
using namespace antlr4;

constexpr int MAX_ENQ = 4;
constexpr int MAX_DEQ = 1;

constexpr int NUM_BUFS = 5;
constexpr int TIMESTEPS = 14;
constexpr int PKT_TYPES = 1;


int main(const int argc, const char *argv[]) {
    if (argc < 2)
        return 1;
    int buf_cap = atoi(argv[1]);
    string model = "fq";
    STSChecker *sts;
    SmtSolver slv;
    sts = new FqChecker(slv, model, NUM_BUFS, TIMESTEPS, PKT_TYPES, buf_cap, MAX_ENQ, MAX_DEQ);
    sts->use_win = false;
    StsRunner runner(sts, model, buf_cap);
    runner.run(NUM_BUFS, TIMESTEPS);
}
