//
// Created by Amir Hossein Seyhani on 10/8/25.
//

#include <format>
#include <ostream>
#include <string>
#include <vector>

#include "utils.hpp"
#include "gen/leaf_workload_parser.hpp"
#include "gen/wl_parser.hpp"

using namespace std;

void add_workload(SmtSolver &slv, ev3 &I, int num_spines, int num_leafs, int host_per_leaf, int timesteps) {
    string wl_file_path = format("./wls/leaf.10.txt");
    vector<vector<string> > wls = read_wl_file(wl_file_path);
    int i = 0;
    auto wl = wls[i];
    string res_stat = wl[0];
    cout << "WL: " << i + 1 << "/" << wls.size() << " " << res_stat << endl;
    LeafWorkloadParser parser(slv, I, num_spines, num_leafs, host_per_leaf, timesteps);
    wl.erase(wl.begin());
    parser.parse(wl);
}


expr valid_meta(ev3 &I, SmtSolver &slv, int buf_idx, int time_idx) {
    ev buf = I[buf_idx][time_idx];
    expr e = slv.ctx.bool_val(true);
    for (int i = 0; i < buf.size(); ++i)
        e = e || buf[i] == sum(buf);
    return e;
}

expr dst_val(ev3 &I, SmtSolver &slv, int buf_idx, int time_idx, int num_buffs) {
    ev buf = I[buf_idx][time_idx];
    expr e = slv.ctx.int_val(-1);
    for (int i = 0; i < buf.size(); ++i)
        e = ite(buf[i] > 0, slv.ctx.int_val(i % num_buffs), e);
    return e;
}

expr ecmp_val(ev3 &I, SmtSolver &slv, int buf_idx, int time_idx, int num_buffs) {
    ev buf = I[buf_idx][time_idx];
    expr e = slv.ctx.int_val(-1);
    for (int i = 0; i < buf.size(); ++i)
        e = ite(buf[i] > 0, slv.ctx.int_val(i / num_buffs), e);
    return e;
}

expr uniq(ev3 &I, SmtSolver &slv, int num_buffs, int timesteps) {
    expr res = slv.ctx.bool_val(true);
    for (int t = 0; t < timesteps; ++t) {
        for (int i = 0; i < num_buffs; ++i) {
            for (int j = 0; j < num_buffs; ++j) {
                if (i == j)
                    continue;
                expr valid_i = valid_meta(I, slv, i, t);
                expr valid_j = valid_meta(I, slv, j, t);
                expr e = implies(valid_i && valid_j,
                                 dst_val(I, slv, i, t, num_buffs) != dst_val(I, slv, i, j, num_buffs));
                res = res && e;
            }
        }
    }
    return res;
}

expr same(ev3 &I, SmtSolver &slv, int num_buffs, int timesteps) {
    expr res = slv.ctx.bool_val(true);
    for (int i = 0; i < num_buffs; ++i) {
        expr init_val = dst_val(I, slv, i, 0, num_buffs);
        res = res && valid_meta(I, slv, i, 0);
        for (int t = 1; t < timesteps; ++t) {
            res = res && ((init_val == dst_val(I, slv, i, t, num_buffs)) && valid_meta(I, slv, i, t));
        }
    }
    return res;
}
