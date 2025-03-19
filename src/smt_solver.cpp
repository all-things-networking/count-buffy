//
// Created by Amir Hossein Seyhani on 3/14/25.
//

#include "smt_solver.hpp"

SmtSolver::SmtSolver(): s(ctx) {
    params p(ctx);
    p.set("random_seed", 100u);
    s.set(p);
}

ev &SmtSolver::bv(const int k, const string &name) {
    const auto result = new vector<expr>[k];
    for (int i = 0; i < k; i++) {
        string vname = name;
        vname += to_string(i);
        expr e = ctx.bool_const(vname.c_str());
        result->push_back(e);
    }
    return *result;
}

evv &SmtSolver::bvv(const int m, const int k, const string &name) {
    auto *result = new vector<ev>[m];
    for (int i = 0; i < m; i++) {
        string vname = name;
        vname += to_string(i);
        auto v = bv(k, vname);
        result->push_back(v);
    }
    return *result;
}

evvv &SmtSolver::bvvv(const int n, const int m, const int k, const string &name) {
    auto *result = new vector<evv>[n];
    for (int i = 0; i < m; i++) {
        string vname = name;
        vname += to_string(i);
        auto vv = bvv(m, k, vname);
        result->push_back(vv);
    }
    return *result;
}

ev &SmtSolver::iv(const int k, const string &name) {
    auto *result = new vector<expr>[k];
    for (int i = 0; i < k; i++) {
        string vname = name;
        vname += "_" + to_string(i);
        expr e = ctx.int_const(vname.c_str());
        result->push_back(e);
    }
    return *result;
}

evv &SmtSolver::ivv(const int m, const int k, const string &name) {
    const auto result = new vector<ev>[m];
    for (int i = 0; i < m; i++) {
        string vname = name;
        vname += "_" + to_string(i);
        auto v = iv(k, vname);
        result->push_back(v);
    }
    return *result;
}

evvv &SmtSolver::ivvv(const int n, const int m, const int k, const string &name) {
    const auto result = new vector<evv>[k];
    for (int i = 0; i < n; i++) {
        string vname = name;
        vname += "_" + to_string(i);
        auto vv = ivv(m, k, vname);
        result->push_back(vv);
    }
    return *result;
}

void SmtSolver::add(const expr &e, const string &name) {
    s.add(e, name.c_str());
}

model SmtSolver::check_sat() {
    switch (s.check()) {
        case sat: cout << "Done!\n";
            return s.get_model();
        default:
            cout << "UNSAT Core:" << endl;
            cout << s.unsat_core() << endl;
            throw runtime_error("Model is not SAT!");
    }
}


void SmtSolver::check_unsat() {
    switch (s.check()) {
        case unsat: cout << "Done!\n";
            break;
        default:
            throw runtime_error("Model is SAT!");
    }
}

void SmtSolver::add_bound(const evvv &vvv, const int lower, const int upper) {
    for (const auto &vv: vvv) {
        for (const auto &v: vv) {
            for (const auto &e: v) {
                s.add((e >= lower) & (e <= upper));
            }
        }
    }
}
