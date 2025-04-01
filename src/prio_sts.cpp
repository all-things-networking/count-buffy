#include "prio_sts.hpp"

PrioSTS::PrioSTS(SmtSolver &slv, const string &var_prefix, int n, int m, int k, int c, int me, int md): STSChecker(slv,
    var_prefix, n, m, k, c, me, md) {
}

expr PrioSTS::workload() {
    expr res = slv.ctx.bool_val(true);
    for (int i = 0; i < timesteps; ++i) {
        res = res && (I[0][i] + I[1][i]) > 0;
        res = res && (I[2][i] > 0);
    }
    return res;
}

expr PrioSTS::out(const ev &bv, const ev &sv, const evv &ov) {
    expr res = slv.ctx.bool_val(true);
    expr not_until = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i) {
        res = res && ite(not_until && bv[i], ov[i] == 1, ov[i] == 0);
        not_until = not_until && (!bv[i]);
    }
    return res;
}

expr PrioSTS::trs(ev const &b, ev const &s, ev const &bp, ev const &sp) {
    expr res = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i) {
        for (int l = i + 1; l < num_bufs; ++l) {
            res = res && (implies(b[i], implies(b[l], bp[l])));
        }
    }
    return res;
}


expr PrioSTS::query(const int p) {
    expr res = slv.ctx.bool_val(false);
    for (int i = 0; i < timesteps - p; ++i) {
        expr part = slv.ctx.bool_val(true);
        for (int j = 0; j < p; ++j) {
            part = part && B[2][i + j];
            part = part && (O[2][i + j] == 0);
        }
        res = res || part;
    }
    return res;
}

expr PrioSTS::init(const ev &b0, const ev &s0) {
    return slv.ctx.bool_val(true);
}
