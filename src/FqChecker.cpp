//
// Created by Amir Hossein Seyhani on 7/28/25.
//

#include "FqChecker.hpp"

#include <support/CPPUtils.h>

#include "IntSeq.hpp"

ev get_new_queue(const ev2 &s) {
    return s[0];
}

ev get_old_queue(const ev2 &s) {
    return s[1];
}


vector<NamedExp> FqChecker::out(const ev &bv, const ev &sv, const ev2 &ov) {
    vector<NamedExp> res;
    // IntSeq nq;
    // IntSeq oq;
    // for (int i = 0; i < num_bufs; ++i) {
    //     expr should_dequeue = (bv[i]) && ((nq.length() > 0 && nq.head() == i) || (
    //                                           nq.length() == 0 && oq.length() > 0 && oq.head() == i));
    //     res.emplace_back(ite(bv[i] && nq.head() == i, ov[i] == 1, ov[i] == 0));
    // }

    return res;
}

vector<NamedExp> FqChecker::trs(const ev &b, const ev &s, const ev &bp, const ev &sp) {
    IntSeq is(&slv.ctx);
    vector<expr> is_head;
    vector<expr> is_selected;

    expr x = s[0];
    // expr nq = is.cr
    // IntSeq nq(x);

    expr y = s[1];
    // IntSeq oq(y);

    for (int i = 0; i < num_bufs; ++i) {
        expr i_selected = b[i] && is_head[i];
        is_selected.push_back(i_selected);
    }

    // context ctx = x.ctx();
    // expr new_nq = IntSeq::create(x.ctx(), "nq_temp");
    // expr new_nq = nq.e;
    for (int i = 0; i < num_bufs; ++i) {
        expr demote = is_selected[i] && bp[i];
        expr deactive = is_selected[i] && !bp[i];
        expr active = !b[i] && bp[i];
        // new_nq = ite(active, nq.push_back(i), nq);
    }
    vector<NamedExp> res;
    return res;
}

vector<NamedExp> FqChecker::init(const ev &b0, const ev &s0) {
    vector<NamedExp> res;
    return res;
}
