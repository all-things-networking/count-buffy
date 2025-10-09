#include "rr_checker.hpp"
#include "lib.hpp"

constexpr int PERIOD = 5;

vector<NamedExp> RRChecker::workload() {
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
    return {NamedExp(base_wl && wl, "workload")};
}


vector<NamedExp> RRChecker::base_wl() {
    expr base_wl = slv.ctx.bool_val(true);
    int period = 5;
    int recur = timesteps / period;
    int rate = period - 1;
    for (int i = 0; i < num_bufs; ++i) {
        for (int j = 1; j <= recur; ++j) {
            base_wl = base_wl && (sum(I[i], j * period - 1) >= rate);
        }
    }
    return {NamedExp(base_wl, "base_wl")};
}

vector<NamedExp> RRChecker::out(const ev &bv, const ev &sv, const ev2 &ov, int t) {
    expr res = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i) {
        // [5]
        res = res && ite(bv[i] && sv[0] == i, ov[i] == 1, ov[i] == 0);
    }
    return {NamedExp(res)};
}

vector<NamedExp> RRChecker::init(const ev &b0, const ev &s0) {
    expr res = slv.ctx.bool_val(true);
    expr not_prevs = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i) {
        res = res && implies(not_prevs && b0[i], s0[0] == i);
        not_prevs = not_prevs && !b0[i];
    }
    res = res && (implies(not_prevs, s0[0] == 0));
    return {NamedExp(res)};
}

const int NUM_BUFS = 5;

expr turn_transition(context &ctx, const vector<expr> &cur_b, const expr &cur_s, const vector<expr> &nxt_b,
                     const expr &nxt_s) {
    expr transition_expr = ctx.bool_val(true);
    for (int i = 0; i < NUM_BUFS; ++i) {
        expr x = ctx.bool_val(true);
        expr y = ctx.bool_val(true);
        for (int j = 1; j < NUM_BUFS; ++j) {
            const int l = (i + j) % NUM_BUFS;
            y = y && implies(x && nxt_b[l], nxt_s == l);
            x = x && !nxt_b[l];
        }
        y = y && implies(x, nxt_s == i);
        transition_expr = transition_expr && implies(cur_s == i, y);
    }

    return transition_expr;
}

vector<NamedExp> RRChecker::trs(const ev &b, const ev &s, const ev &bp, const ev &sp, int tp) {
    assert(b.size() == num_bufs);
    assert(s.size() == num_bufs);

    expr e = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i) {
        expr x = slv.ctx.int_val(i);
        for (int j = 1; j < num_bufs; ++j) {
            const int l = (i - j + num_bufs) % num_bufs;
            x = ite(bp[l], slv.ctx.int_val(l), x);
        }
        e = e && implies(s[0] == i, sp[0] == x);
    }

    return {e};
}

vector<NamedExp> RRChecker::query() {
    expr res = slv.ctx.bool_val(true);
    // The following is based on FPerf not the FPerf paper
    for (int i = timesteps - PERIOD; i < timesteps; ++i) {
        res = res && (sum(O[2], i) - sum(O[1], i) >= 3);
    }
    return {NamedExp(res)};
}
