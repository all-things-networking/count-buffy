#include "prio_sts.hpp"

PrioSTS::PrioSTS(SmtSolver &slv, const string &var_prefix, int n, int m, int k, int c, int me, int md): STSChecker(slv,
    var_prefix, n, m, k, c, me, md) {
}

vector<NamedExp> PrioSTS::workload() {
    vector<NamedExp> res;
    for (int i = 0; i < timesteps - 1; ++i)
        res.emplace_back((E[0][i] + E[1][i]) > 0, format("wl_s(0,1)[{}]", i));

    expr b2_constr = E[2][0] > 0;
    for (int i = 0; i < timesteps - 5; ++i) {
        b2_constr = b2_constr || E[2][0] > 0;
    }
    res.emplace_back(b2_constr, "wl_2[0]");
    return res;
}

vector<NamedExp> PrioSTS::out(const ev &bv, const ev &sv, const ev2 &ov) {
    vector<NamedExp> rv;
    expr res = slv.ctx.bool_val(true);
    expr not_until = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i) {
        res = res && ite(not_until && bv[i], ov[i] == 1, ov[i] == 0);
        not_until = not_until && (!bv[i]);
        rv.emplace_back(res, format("out[{}]", i));
    }
    return rv;
}

vector<NamedExp> PrioSTS::trs(ev const &b, ev const &s, ev const &bp, ev const &sp) {
    vector<NamedExp> rv;
    expr res = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i) {
        for (int l = i + 1; l < num_bufs; ++l) {
            res = res && (implies(b[i], implies(b[l], bp[l])));
            rv.emplace_back(res, format("trs[{}][{}]", i, l));
        }
    }
    return rv;
}


vector<NamedExp> PrioSTS::query(const int p) {
    expr res = slv.ctx.bool_val(false);
    for (int i = 0; i < timesteps - p; ++i) {
        expr part = slv.ctx.bool_val(true);
        for (int j = 0; j < p + 1; ++j) {
            part = part && B[2][i + j];
            part = part && (O[2][i + j] == 0);
        }
        res = res || part;
    }
    return {NamedExp(res, "query")};
}

vector<NamedExp> PrioSTS::init(const ev &b0, const ev &s0) {
    return {NamedExp(slv.ctx.bool_val(true), "init")};
}
