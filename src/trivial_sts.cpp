#include "trivial_sts.hpp"
#include "lib.hpp"


vector<NamedExp> TrivialSts::workload() {
    expr base_wl = slv.ctx.bool_val(true);
    return {NamedExp(base_wl, "workload")};
}

vector<NamedExp> TrivialSts::out(const ev &bv, const ev &sv, const ev2 &ov) {
    expr res = slv.ctx.bool_val(true);
    return {NamedExp(res, "out")};
}

vector<NamedExp> TrivialSts::init(const ev &b0, const ev &s0) {
    expr res = slv.ctx.bool_val(true);
    return {NamedExp(res, "init")};
}


vector<NamedExp> TrivialSts::trs(const ev &b, const ev &s, const ev &bp, const ev &sp) {
    expr res = slv.ctx.bool_val(true);
    return {NamedExp(res, "trs")};
}

vector<NamedExp> TrivialSts::query(int m) {
    expr res = slv.ctx.bool_val(true);
    return {NamedExp(res, "query")};
}
