#include "rr_checker.hpp"
#include <cmath>
#include "lib.hpp"

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

expr RRChecker::bar(const ev &b, const ev &s, const ev &bp, const ev &sp, int tp) {
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

    return e;
}

vector<NamedExp> RRChecker::trs(const ev &b, const ev &s, const ev &bp, const ev &sp, int tp) {
    return {bar(b, s, bp, sp, tp)};
    assert(b.size() == num_bufs);
    assert(s.size() == num_bufs);

    const expr &state = sp[0];

    expr next_turn = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i) {
        expr not_until = slv.ctx.bool_val(true);
        expr next_backlogged = slv.ctx.bool_val(true);
        for (int j = 1; j < num_bufs; ++j) {
            const int l = (i + j) % num_bufs;
            next_backlogged = next_backlogged && implies((s[0] == i) && not_until && bp[l], state == l);
            not_until = not_until && !bp[l];
        }
        next_backlogged = next_backlogged && implies(not_until, sp[0] == i);
        next_turn = next_turn && implies(s[0] == i, next_backlogged);
        // expr not_until = slv.ctx.bool_val(true);
        // expr first_backlog = slv.ctx.bool_val(true);
        // for (int l = 1; l <= num_bufs; ++l) {
        // const int j = (i + l) % num_bufs;
        // first_backlog = first_backlog && ite(not_until && bp[j], sp[j], !sp[j]);
        // not_until = not_until && !bp[j];
        // }
        // next_turn = next_turn && implies(s[i], first_backlog);
    }

    expr max_deq = slv.ctx.bool_val(true);
    // for (int i = 0; i < num_bufs; ++i) {
    // max_deq = max_deq && implies(!s[i], implies(b[i], bp[i]));
    // }
    return {NamedExp(next_turn && max_deq)};
}

vector<NamedExp> RRChecker::query(int m) {
    expr res = slv.ctx.bool_val(true);
    for (int i = m; i < timesteps; ++i) {
        res = res && (sum(O[2], i) - sum(O[1], i) >= 3);
    }
    return {NamedExp(res)};
}
