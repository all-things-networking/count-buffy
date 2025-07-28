//
// Created by Amir Hossein Seyhani on 4/3/25.
//

#include "merger.hpp"


vector<NamedExp> Merger::out(const ev &bv, const ev &sv, const ev2 &ov) {
    vector<NamedExp> res;
    for (int i = 0; i < num_bufs; ++i)
        res.emplace_back(implies(bv[i], ov[i] == 1));
    return res;
}

vector<NamedExp> Merger::trs(ev const &b, ev const &s, ev const &bp, ev const &sp) {
    vector<NamedExp> res;
    return res;
}

vector<NamedExp> Merger::init(ev const &b0, ev const &s0) {
    vector<NamedExp> res;
    return res;
}

vector<NamedExp> Merger::query(int m) {
    vector<NamedExp> res;
    return res;
}

vector<NamedExp> Merger::workload() {
    vector<NamedExp> res;
    return res;
}
