//
// Created by Amir Hossein Seyhani on 6/3/25.
//

#ifndef CLASSIFIER_HPP
#define CLASSIFIER_HPP
#include "smt_solver.hpp"


class Classifier {
public:
    virtual ~Classifier() = default;

    SmtSolver &slv;
    string var_prefix;
    ev3 I;
    ev3 O;
    ev2 match;
    int timesteps;
    int out_bufs;
    int pkt_types;

    Classifier(SmtSolver &slv, const string &var_prefix, int m, int k, int o);

    vector<NamedExp> set_out();

    void print(model m) const;
};


#endif //CLASSIFIER_HPP
