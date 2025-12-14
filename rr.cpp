#include <iostream>

#include <antlr4-runtime.h>
#include <z3++.h>
#include "src/params.hpp"
#include "src/rr_checker.hpp"
#include "src/sts_runner.hpp"
#include "src/gen/constr_extractor.hpp"

class fperfVisitor;
using namespace std;
using namespace z3;
using namespace antlr4;

constexpr int TIMESTEPS = 10;
constexpr int NUM_BUFS = 5;
constexpr int PKT_TYPES = 1;


int main(const int argc, const char *argv[]) {
    if (argc < 2)
        return 1;
    int buf_cap = atoi(argv[1]);
    string model = "rr";
    STSChecker *sts;
    SmtSolver slv;
    sts = new RRChecker(slv, model, NUM_BUFS, TIMESTEPS, PKT_TYPES, buf_cap, MAX_ENQ, MAX_DEQ);
    sts->use_win = false;
    StsRunner runner(sts, model, buf_cap);
    runner.run(NUM_BUFS, TIMESTEPS);
}
