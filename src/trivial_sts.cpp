#include "trivial_sts.hpp"

expr TrivialSts::init(const ev &b0, const ev &s0) {
    return slv.ctx.bool_val(true);
}

expr TrivialSts::query(const int p) {
    return slv.ctx.bool_val(true);
}

expr TrivialSts::trs(ev const &b, ev const &s, ev const &bp, ev const &sp) {
    return slv.ctx.bool_val(true);
}

expr TrivialSts::out(const ev &bv, const ev &sv, const ev2 &ov) {
    return slv.ctx.bool_val(true);
}

expr TrivialSts::workload() {
    return slv.ctx.bool_val(true);
}