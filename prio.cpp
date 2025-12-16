#include <iostream>

#include "antlr4-runtime.h"
#include"z3++.h"
#include "src/params.hpp"
#include "src/prio_sts.hpp"
#include "src/rr_checker.hpp"
#include "src/gen/constr_extractor.hpp"
#include "src/sts_runner.hpp"

using namespace std;
using namespace z3;
using namespace antlr4;

constexpr int TIMESTEPS = 7;
constexpr int NUM_BUFS = 4;
constexpr int PKT_TYPES = 1;


int main(const int argc, const char *argv[]) {
    if (argc < 3)
        throw invalid_argument("Invalid arguments");
    int buf_cap = atoi(argv[1]);
    bool use_win = (string(argv[2]) == "win");
    string model = "prio";
    cout << "Starting experiment [" << model << "] " << "Buffer size = " << buf_cap
            << ", windows = " << use_win << endl;
    PrioSTS *sts;
    SmtSolver slv;
    sts = new PrioSTS(slv, model, NUM_BUFS, TIMESTEPS, PKT_TYPES, buf_cap, MAX_ENQ, MAX_DEQ);
    sts->use_win = use_win;
    StsRunner runner(sts, model, buf_cap);
    runner.run(NUM_BUFS, TIMESTEPS);
}
