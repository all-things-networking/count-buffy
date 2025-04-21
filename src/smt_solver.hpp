//
// Created by Amir Hossein Seyhani on 3/14/25.
//

#ifndef SMT_SOLVER_HPP
#define SMT_SOLVER_HPP

#include <iostream>
#include <sstream>
#include<vector>
#include"z3++.h"
#include "lib.hpp"

using namespace std;
using namespace z3;


class SmtSolver {
    public:
    context ctx;
    solver s;

    SmtSolver();

    ev &bv(int k, const string &name);

    ev2 &bvv(int m, int k, const string &name);

    ev3 &bvvv(int n, int m, int k, const string &name);

    ev &iv(int k, const string &name);

    ev2 &ivv(int m, int k, const string &name);

    ev3 &ivvv(int n,int m, int k, const string &name);

    void add(const expr &e, const string &name);

    void add_bound(const ev3 &vv, int lower, int upper);

    model check_sat();

    void check_unsat();
};

#endif //SMT_SOLVER_HPP
