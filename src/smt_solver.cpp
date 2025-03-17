//
// Created by Amir Hossein Seyhani on 3/14/25.
//

#include "smt_solver.hpp"

SmtSolver::SmtSolver(): s(ctx) {
    params p(ctx);
    p.set("random_seed", 100u);
    s.set(p);
}

vector<expr> &SmtSolver::bool_vector(int n, string name) {
    vector<expr> *result = new vector<expr>[n];
    for (int i = 0; i < n; i++) {
        string vname = name;
        vname += to_string(i);
        expr e = ctx.bool_const(vname.c_str());
        result->push_back(e);
    }
    return *result;
}

vector<ev> &SmtSolver::bool_vectors(int k, int n, string name) {
    vector<ev> *result = new vector<ev>[k];
    for (int i = 0; i < k; i++) {
        string vname = name;
        vname += to_string(i);
        auto bv = bool_vector(n, vname);
        result->push_back(bv);
    }
    return *result;
}

vector<expr> &SmtSolver::int_vector(int n, string name) {
    vector<expr> *result = new vector<expr>[n];
    for (int i = 0; i < n; i++) {
        string vname = name;
        vname += "_" + to_string(i);
        expr e = ctx.int_const(vname.c_str());
        result->push_back(e);
    }
    return *result;
}

vector<ev> &SmtSolver::int_vectors(int k, int n, string name) {
    vector<ev> *result = new vector<ev>[k];
    for (int i = 0; i < k; i++) {
        string vname = name;
        vname += "_" + to_string(i);
        auto iv = int_vector(n, vname);
        result->push_back(iv);
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
        // cout << s.unsat_core() << endl;
            break;
        default:
            throw runtime_error("Model is SAT!");
    }
}

void SmtSolver::add_bound(evv vv, int lower, int upper) {
    for (auto v: vv) {
        for (auto e: v) {
            s.add((e >= lower) & (e <= upper));
        }
    }
}
