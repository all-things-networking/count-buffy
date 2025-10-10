#include <fstream>

#include"z3++.h"
#include "src/classifier.hpp"
#include "src/loom_sts_runner.hpp"
#include "src/rr_checker.hpp"
#include "src/merger.hpp"
#include "src/params.hpp"
#include "src/prio_sts.hpp"

using namespace std;
using namespace z3;

const int TIME_STEPS = 10;
const int RR_IN_BUFS = 2;
const int PKT_TYPES = 3;

class Composed {
public:
    ev3 I;
    ev2 O;
    STSChecker *rr1;
    STSChecker *rr2;
    STSChecker *rr3;
    STSChecker *rr4;
    STSChecker *prio;
    STSChecker *merger;
    Classifier *classifier;
    SmtSolver slv;
    int buffer_size;
    int random_seed;

public:
    Composed(int buffer_size, unsigned int random_seed): slv(random_seed) {
        this->buffer_size = buffer_size;
        this->random_seed = random_seed;
        I = slv.ivvv(4 * PKT_TYPES, TIME_STEPS, PKT_TYPES, "INS");
        slv.add_bound(I, 0, MAX_ENQ);

        rr1 = new RRChecker(slv, "rr1", RR_IN_BUFS, TIME_STEPS, PKT_TYPES, buffer_size, MAX_ENQ, MAX_DEQ);
        rr2 = new RRChecker(slv, "rr2", RR_IN_BUFS, TIME_STEPS, PKT_TYPES, buffer_size, MAX_ENQ, MAX_DEQ);
        rr3 = new RRChecker(slv, "rr3", RR_IN_BUFS, TIME_STEPS, PKT_TYPES, buffer_size, MAX_ENQ, MAX_DEQ);
        rr4 = new RRChecker(slv, "rr4", RR_IN_BUFS, TIME_STEPS, PKT_TYPES, buffer_size, MAX_ENQ, MAX_DEQ);
        vector rrs = {rr1, rr2, rr3, rr4};
        for (int i = 0; i < rrs.size(); ++i) {
            rrs[i]->I[0] = I[i * 3];
            rrs[i]->I[1] = I[i * 3 + 1] + I[i * 3 + 2];
            auto constrs = rrs[i]->base_constrs();
            slv.add(merge(constrs, format("rr{}_base", i)));
        }

        merger = new Merger(slv, "mg", 4, TIME_STEPS, PKT_TYPES, buffer_size, MAX_ENQ, MAX_DEQ);
        for (int i = 0; i < rrs.size(); ++i)
            merger->I[i] = rrs[i]->O[0] + rrs[i]->O[1];
        slv.add(merge(merger->base_constrs(), "merger_base"));

        classifier = new Classifier(slv, "cls", TIME_STEPS, PKT_TYPES, 2);
        classifier->I[0] = merger->O[0] + merger->O[1] + merger->O[2] + merger->O[3];
        classifier->set_out();

        prio = new PrioSTS(slv, "prio", 2, TIME_STEPS, PKT_TYPES, buffer_size, MAX_ENQ, MAX_DEQ);
        prio->I[0] = classifier->O[0];
        prio->I[1] = classifier->O[1];
        slv.add(merge(prio->base_constrs(), "prio_base"));

        O = prio->O[0] + prio->O[1];
    }
};

int main(const int argc, const char *argv[]) {
    int buff_cap = stoi(argv[1]);
    Composed c(buff_cap, 50000);
    LoomStsRunner runner(c.slv, c.I, c.O, "loom_mem", buff_cap);
    runner.run();
}
