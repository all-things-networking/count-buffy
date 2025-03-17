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

    ev &bool_vector(int n, const string &name);

    evv &bool_vectors(int k, int, const string &name);

    ev &int_vector(int n, const string &name);

    evv &int_vectors(int k, int, const string &name);

    void add(const expr &e, const string &name);

    void add_bound(const evv &vv, int lower, int upper);

    model check_sat();

    void check_unsat();
};

#endif //SMT_SOLVER_HPP
