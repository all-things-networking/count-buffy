#include "prio_sts.hpp"

PrioSTS::PrioSTS(SmtSolver &slv, const string &var_prefix, int n, int m, int k, int c, int me, int md): STSChecker(slv,
    var_prefix, n, m, k, c, me, md) {
}

vector<NamedExp> PrioSTS::workload() {
    vector<NamedExp> res;
    for (int i = 0; i < timesteps - 1; ++i)
        res.emplace_back((E[0][i] + E[1][i]) > 0);

    expr b2_constr = E[2][0] > 0;
    for (int i = 0; i < timesteps - 5; ++i) {
        b2_constr = b2_constr || E[2][0] > 0;
    }
    res.emplace_back(b2_constr);
    return res;
}

vector<NamedExp> PrioSTS::out(const ev &bv, const ev &sv, const ev2 &ov, int t) {
    vector<NamedExp> rv;
    expr res = slv.ctx.bool_val(true);
    expr not_until = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i) {
        res = res && ite(not_until && bv[i], ov[i] == 1, ov[i] == 0);
        not_until = not_until && (!bv[i]);
        rv.emplace_back(res);
    }
    return rv;
}

vector<NamedExp> PrioSTS::trs(ev const &b, ev const &s, ev const &bp, ev const &sp, int tp) {
    vector<NamedExp> rv;
    expr res = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i) {
        for (int l = i + 1; l < num_bufs; ++l) {
            res = res && (implies(b[i], implies(b[l], bp[l])));
            rv.emplace_back(res);
        }
    }
    return rv;
}


constexpr int QUERY_TRESH = 6;

vector<NamedExp> PrioSTS::query() {
    expr res = slv.ctx.bool_val(false);
    for (int i = 0; i < timesteps - QUERY_TRESH + 1; ++i) {
        expr part = slv.ctx.bool_val(true);
        for (int j = 0; j < QUERY_TRESH; ++j) {
            part = part && B[2][i + j];
            part = part && (O[2][i + j] == 0);
        }
        res = res || part;
    }
    return {res};
}

vector<NamedExp> PrioSTS::init(const ev &b0, const ev &s0) {
    return {slv.ctx.bool_val(true)};
}
