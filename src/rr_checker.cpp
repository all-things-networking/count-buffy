//
// Created by Amir Hossein Seyhani on 3/17/25.
//

#include "rr_checker.hpp"
#include "lib.hpp"

expr RRChecker::workload() {
    expr base_wl = slv.ctx.bool_val(true);
    int period = 5;
    int recur = timesteps / period;
    int rate = period - 1;
    for (int i = 0; i < num_bufs; ++i) {
        for (int j = 1; j <= recur; ++j) {
            base_wl = base_wl && (sum(I[i], j * period) >= rate);
        }
    }
    base_wl = base_wl && (sum(I[1]) >= sum(I[2]));
    expr wl = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i) {
        if (i == 2) {
            for (int j = 0; j < period; ++j) {
                wl = wl & (I[i][j] > 0);
            }
        } else {
            wl = wl & (sum(I[i], period - 1) == 0);
        }
    }
    return base_wl && wl;
}

expr RRChecker::out(const ev &bv, const ev &sv, const ev2 &ov) {
    expr res = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i) {
        res = res && ite(bv[i] && sv[i], ov[i] == 1, ov[i] == 0);
    }
    return res;
}

expr RRChecker::init(const ev &b0, const ev &s0) {
    expr res = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i) {
        if (i == 0)
            res = res && s0[i];
        else
            res = res && !s0[i];
    }
    return res;
}


expr RRChecker::trs(const ev &b, const ev &s, const ev &bp, const ev &sp) {
    assert(b.size() == num_bufs);
    assert(s.size() == num_bufs);

    expr next_turn = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i) {
        expr not_until = slv.ctx.bool_val(true);
        expr first_backlog = slv.ctx.bool_val(true);
        for (int l = 1; l <= num_bufs; ++l) {
            const int j = (i + l) % num_bufs;
            first_backlog = first_backlog && ite(not_until && bp[j], sp[j], !sp[j]);
            not_until = not_until && !bp[j];
        }
        next_turn = next_turn && implies(s[i], first_backlog);
    }

    expr max_deq = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i) {
        max_deq = max_deq && implies(!s[i], implies(b[i], bp[i]));
    }
    return next_turn && max_deq;
}

expr RRChecker::query(int m) {
    expr res = slv.ctx.bool_val(true);
    res = (sum(O[2]) - sum(O[1])) >= 3;
    return res;
}
