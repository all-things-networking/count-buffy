//
// Created by Amir Hossein Seyhani on 3/17/25.
//

#include "rr_checker.hpp"

expr RRChecker::workload() {
    expr base_wl = slv.ctx.bool_val(true);
    int period = 5;
    int recur = n / period;
    int rate = period - 1;
    for (int j = 0; j < k; ++j) {
        for (int i = 1; i <= recur; ++i) {
            base_wl = base_wl && (sum(I[j], i * period) >= rate);
        }
    }
    base_wl = base_wl && (sum(I[1]) >= sum(I[2]));
    expr wl = slv.ctx.bool_val(true);
    for (int j = 0; j < k; ++j) {
        if (j == 2) {
            for (int i = 0; i < period; ++i) {
                wl = wl & (I[j][i] > 0);
            }
        } else {
            wl = wl & (sum(I[j], period - 1) == 0);
        }
    }
    return base_wl && wl;
}

expr RRChecker::out(const ev &bv, const ev &sv, const ev &ov) {
    expr res = slv.ctx.bool_val(true);
    for (int i = 0; i < k; ++i) {
        res = res && ite(bv[i] && sv[i], ov[i] == 1, ov[i] == 0);
    }
    return res;
}

expr RRChecker::init(const ev &b0, const ev &s0) {
    expr res = slv.ctx.bool_val(true);
    for (int i = 0; i < k; ++i) {
        if (i == 0)
            res = res && s0[i];
        else
            res = res && !s0[i];
    }
    return res;
}


expr RRChecker::trs(const ev &b, const ev &s, const ev &bp, const ev &sp) {
    assert(b.size() == k);
    assert(s.size() == k);

    expr next_turn = slv.ctx.bool_val(true);
    for (int i = 0; i < k; ++i) {
        expr not_until = slv.ctx.bool_val(true);
        expr first_backlog = slv.ctx.bool_val(true);
        for (int l = 1; l <= k; ++l) {
            const int j = (i + l) % k;
            first_backlog = first_backlog && ite(not_until && bp[j], sp[j], !sp[j]);
            not_until = not_until && !bp[j];
        }
        next_turn = next_turn && implies(s[i], first_backlog);
    }

    expr max_deq = slv.ctx.bool_val(true);
    for (int i = 0; i < k; ++i) {
        max_deq = max_deq && implies(!s[i], implies(b[i], bp[i]));
    }
    return next_turn && max_deq;
}

expr RRChecker::sum(const ev &v) {
    expr res = slv.ctx.int_val(0);
    for (const auto &e: v)
        res = res + e;
    return res;
}

expr RRChecker::sum(const ev &v, int limit) {
    expr res = slv.ctx.int_val(0);
    for (int i = 0; i < limit; ++i) {
        res = res + v[i];
    }
    return res;
}

expr RRChecker::query(int m) {
    expr res = slv.ctx.bool_val(true);
    res = (sum(O[2]) - sum(O[1])) >= 3;
    return res;
}
