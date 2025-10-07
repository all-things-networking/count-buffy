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


int main(const int argc, const char *argv[]) {
    if (argc < 5)
        return 1;
    int num_buffers = atoi(argv[1]);
    int timesteps = atoi(argv[2]);
    int pkt_types = atoi(argv[3]);
    int buf_cap = atoi(argv[4]);
    string model = "prio";
    PrioSTS *sts;
    SmtSolver slv;
    sts = new PrioSTS(slv, model, num_buffers, timesteps, pkt_types, buf_cap, MAX_ENQ, MAX_DEQ);
    sts->use_win = true;
    StsRunner runner(sts, model, buf_cap);
    runner.run(num_buffers, timesteps);
}
