//
// Created by Amir Hossein Seyhani on 10/10/25.
//

#ifndef LOOM_STS_RUNNER_HPP
#define LOOM_STS_RUNNER_HPP
#include "smt_solver.hpp"


class LoomStsRunner {
private:
    int buff_cap;
    string model;
    SmtSolver &slv;
    ev3 &I;
    ev2 &O;

public:
    LoomStsRunner(SmtSolver &slv, ev3&I, ev2& O, string model, int buff_cap);

    vector<NamedExp> base_wl(SmtSolver &slv, const ev3 &ins);

    vector<NamedExp> query(SmtSolver &slv, ev2 &out);

    void run();
};


#endif //LOOM_STS_RUNNER_HPP
