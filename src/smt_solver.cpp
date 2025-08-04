//
// Created by Amir Hossein Seyhani on 3/14/25.
//

#include "smt_solver.hpp"

#include <fstream>
#include <set>

#include "IntSeq.hpp"

SmtSolver::SmtSolver(): s(ctx) {
    params p(ctx);
    p.set("random_seed", 600u);
    s.set(p);
}

SmtSolver::SmtSolver(unsigned int random_seed): s(ctx) {
    params p(ctx);
    p.set("random_seed", random_seed);
    s.set(p);
}

ev &SmtSolver::bv(const int k, const string &name) {
    const auto result = new vector<expr>[k];
    for (int i = 0; i < k; i++) {
        string vname = name;
        vname += "_" + to_string(i);
        expr e = ctx.bool_const(vname.c_str());
        result->push_back(e);
    }
    return *result;
}

ev2 &SmtSolver::bvv(const int m, const int k, const string &name) {
    auto *result = new vector<ev>[m];
    for (int i = 0; i < m; i++) {
        string vname = name;
        vname += "_" + to_string(i);
        auto v = bv(k, vname);
        result->push_back(v);
    }
    return *result;
}

ev3 &SmtSolver::bvvv(const int n, const int m, const int k, const string &name) {
    auto *result = new vector<ev2>[n];
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

ev &SmtSolver::sv(const int k, const string &name) {
    IntSeq is(&ctx);

    auto *result = new vector<expr>[k];
    for (int i = 0; i < k; i++) {
        string vname = name;
        vname += "_" + to_string(i);
        expr e = is.create(vname);
        result->push_back(e);
    }
    return *result;
}

ev2 &SmtSolver::svv(const int m, const int k, const string &name) {
    const auto result = new vector<ev>[m];
    for (int i = 0; i < m; i++) {
        string vname = name;
        vname += "_" + to_string(i);
        auto v = sv(k, vname);
        result->push_back(v);
    }
    return *result;
}

ev2 &SmtSolver::ivv(const int m, const int k, const string &name) {
    const auto result = new vector<ev>[m];
    for (int i = 0; i < m; i++) {
        string vname = name;
        vname += "_" + to_string(i);
        auto v = iv(k, vname);
        result->push_back(v);
    }
    return *result;
}

ev3 &SmtSolver::ivvv(const int n, const int m, const int k, const string &name) {
    const auto result = new vector<ev2>[k];
    for (int i = 0; i < n; i++) {
        string vname = name;
        vname += "_" + to_string(i);
        auto vv = ivv(m, k, vname);
        result->push_back(vv);
    }
    return *result;
}

ev SmtSolver::const_vec(int size, int val) {
    ev result;
    for (int i = 0; i < size; ++i) {
        result.push_back(ctx.int_val(val));
    }
    return result;
}

void SmtSolver::add(const expr &e, const string &prefix) {
    add({e, format("{}_{}", prefix, e.to_string())});
}

void SmtSolver::add(const NamedExp &ne) {
    try {
        s.add(ne.e, ne.name.c_str());
    } catch (z3::exception &e) {
        cout << "Error: " << e.msg() << " " << ne.name << endl;
        throw;
    }
}

void SmtSolver::add(const vector<NamedExp> &nes) {
    for (const auto &ne: nes)
        add(ne);
}


model SmtSolver::check_sat() {
    // ofstream out("stats.log", ios::app);
    switch (s.check()) {
        case sat:
            // cout << "Done!\n";
            // cout << s.statistics() << endl;
            return s.get_model();
        default:
            // cout << s.statistics() << endl;
            cout << s.unsat_core() << endl;
            throw runtime_error("Model is not SAT!");
    }
}


void SmtSolver::check_unsat() {
    switch (s.check()) {
        case unsat:
            // cout << "Done!\n";
            break;
        default:
            throw runtime_error("Model is SAT!");
    }
    // cout << s.statistics() << endl;
}


void SmtSolver::add_bound(const ev3 &vvv, const int lower, const int upper) {
    expr res = ctx.bool_val(true);
    for (const auto &vv: vvv) {
        for (const auto &v: vv) {
            for (const auto &e: v) {
                res = res && (e >= lower) && (e <= upper);
            }
        }
    }
    s.add(res);
}

pair<ev, ev> SmtSolver::capped(const ev &v, int cap) {
    auto s = ctx.int_val(0);
    auto C = ctx.int_val(cap);
    ev capped;
    ev slack;
    for (int i = 0; i < v.size(); ++i) {
        auto val = ite(s + v[i] <= C, v[i], ite(s == C, ctx.int_val(0), C - s));
        slack.push_back(v[i] - val);
        capped.push_back(val);
        s = s + val;
    }
    return {capped, slack};
}

string SmtSolver::stats_str() {
    stringstream ss;

    ss << "z3 statistics: " << endl;
    stats sts = s.statistics();
    for (unsigned int i = 0; i < sts.size(); i++) {
        ss << sts.key(i) << ": ";
        if (sts.is_uint(i)) {
            ss << sts.uint_value(i) << endl;
        } else if (sts.is_double(i)) {
            ss << sts.double_value(i) << endl;
        }
    }

    return ss.str();
}
