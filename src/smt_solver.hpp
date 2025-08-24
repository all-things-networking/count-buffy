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
#include "named_expr.hpp"

using namespace std;
using namespace z3;


class SmtSolver {
public:
    context ctx;
    solver s;

    SmtSolver();

    SmtSolver(unsigned int random_seed);

    ev &bv(int k, const string &name);

    ev2 &bvv(int m, int k, const string &name);

    ev3 &bvvv(int n, int m, int k, const string &name);

    ev &iv(int k, const string &name);

    ev &sv(int k, const string &name);

    ev2 &svv(int m, int k, const string &name);

    ev2 &ivv(int m, int k, const string &name);

    ev3 &ivvv(int n, int m, int k, const string &name);

    ev const_vec(int size, int val);

    void add(const expr &e, const string &prefix);

    void add(const NamedExp &ne);

    void add(const vector<NamedExp> &nes);

    void add_bound(const ev3 &vv, int lower, int upper);

    void add_bound(const ev2 &vv, int lower, int upper);

    pair<ev, ev> capped(const ev &v, int cap);

    model check_sat();

    void check_unsat();

    string stats_str();
};

#endif //SMT_SOLVER_HPP
