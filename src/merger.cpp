//
// Created by Amir Hossein Seyhani on 4/3/25.
//

#include "merger.hpp"


expr Merger::out(const ev &bv, const ev &sv, const ev2 &ov) {
    expr res = slv.ctx.bool_val(true);
    expr not_until = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i) {
        res = res && ite(not_until && bv[i], ov[i] == 1, ov[i] == 0);
        not_until = not_until && (!bv[i]);
    }
    return res;
}

expr Merger::trs(ev const &b, ev const &s, ev const &bp, ev const &sp) {
    expr res = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i) {
        for (int l = i + 1; l < num_bufs; ++l) {
            res = res && (implies(b[i], implies(b[l], bp[l])));
        }
    }
    return res;
}

expr Merger::init(ev const &b0, ev const &s0) {

}
