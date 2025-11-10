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
    int bool_vars;
    int int_vars;

    SmtSolver();

    SmtSolver(unsigned int random_seed);

    ev &bv(int k, const string &name);

    ev &bv(int k, const bool &val);

    ev2 &bvv(int m, int k, const string &name);

    ev3 &bvvv(int n, int m, int k, const string &name);

    ev &iv(int k, const string &name);

    ev &iv(int k, int const_val);

    ev &iv(int k, const string &name, vector<int> pkt_types);

    ev &sv(int k, const string &name);

    ev2 &svv(int m, int k, const string &name);

    ev2 &ivv(int m, int k, const string &name);

    ev2 &ivv(int m, int k, int const_val);

    ev2 &ivv(int m, int k, const string &name, vector<int> pkt_types);

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

    void print_stats();
};

#endif //SMT_SOLVER_HPP
