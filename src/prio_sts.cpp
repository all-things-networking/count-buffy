#include "prio_sts.hpp"

PrioSTS::PrioSTS(int n, int k, int c, int me, int md): STSChecker(n, k, c, me, md) {
}

expr PrioSTS::workload() {
    expr res = slv.ctx.bool_val(true);
    for (int i = 0; i < n; ++i) {
        res = res && (I[0][i] + I[1][i]) > 0;
        res = res && (I[2][i] > 0);
    }
    return res;
}

expr PrioSTS::out(const ev &bv, const ev &sv, const ev &ov) {
    expr res = slv.ctx.bool_val(true);
    expr not_until = slv.ctx.bool_val(true);
    for (int i = 0; i < k; ++i) {
        res = res && ite(not_until && bv[i], ov[i] == slv.ctx.int_val(1), ov[i] == slv.ctx.int_val(0));
        not_until = not_until && (!bv[i]);
    }
    return res;
}

expr PrioSTS::trs(ev const &b, ev const &s, ev const &bp, ev const &sp) {
    expr res = slv.ctx.bool_val(true);
    for (int j = 0; j < k; ++j) {
        for (int l = j + 1; l < k; ++l) {
            res = res && (implies(b[j], implies(b[l], bp[l])));
        }
    }
    return res;
}


expr PrioSTS::query(const int m) {
    expr res = slv.ctx.bool_val(false);
    for (int i = 0; i < n - m; ++i) {
        expr part = slv.ctx.bool_val(true);
        for (int j = 0; j < m; ++j) {
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
