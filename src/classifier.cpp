//
// Created by Amir Hossein Seyhani on 6/3/25.
//

#include "classifier.hpp"

Classifier::Classifier(SmtSolver &slv, const string &var_prefix, int m, int k, int o) : slv(slv),
    var_prefix(move(var_prefix)), timesteps(m), out_bufs(o), pkt_types(k) {
    I = slv.ivvv(1, m, k, format("I_{}", var_prefix));
    O = slv.ivvv(2, m, k, format("O_{}", var_prefix));
    slv.add_bound(I, 0, 100);
    slv.add_bound(O, 0, 100);
}

vector<NamedExp> Classifier::set_out() {
    vector<NamedExp> nes;
    // 0,1 -> 1
    // 2 -> 0
    for (int j = 0; j < timesteps; ++j) {
        O[1][j][0] = I[0][j][0];
        O[1][j][1] = I[0][j][1];
        O[1][j][2] = slv.ctx.int_val(0);
        O[0][j][0] = slv.ctx.int_val(0);
        O[0][j][1] = slv.ctx.int_val(0);
        O[0][j][2] = I[0][j][2];
    }
    return nes;
}
