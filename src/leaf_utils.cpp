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

void add_workload(SmtSolver &slv, ev3 &I, int timesteps, map<int, int> pkt_type_to_dst,
                  map<int, int> pkt_type_to_ecmp) {
    string wl_file_path = format("./leaf.txt");
    vector<vector<string> > wls = read_wl_file(wl_file_path);
    int i = 0;
    auto wl = wls[i];
    string res_stat = wl[0];
    cout << "WL: " << i + 1 << "/" << wls.size() << " " << res_stat << endl;
    LeafWorkloadParser parser(slv, I, timesteps, pkt_type_to_dst, pkt_type_to_ecmp);
    wl.erase(wl.begin());
    parser.parse(wl);
}


expr valid_meta(ev3 &I, SmtSolver &slv, int buf_idx, int time_idx) {
    ev buf = I[buf_idx][time_idx];
    int pkt_types = buf.size();
    // All empty
    expr e = slv.ctx.bool_val(true);
    for (int j = 0; j < pkt_types; ++j) {
        e = e && (buf[j] == 0);
    }
    for (int i = 0; i < pkt_types; ++i) {
        expr others_empty = slv.ctx.bool_val(true);
        for (int j = 0; j < pkt_types; ++j) {
            if (i == j)
                continue;
            others_empty = others_empty && (buf[j] == 0);
        }
        e = ite(buf[i] > 0, others_empty, e);
    }
    return e;
}

expr dst_val(ev3 &I, SmtSolver &slv, map<int, vector<int> > dst_to_pkt_type, int buf_idx, int time_idx) {
    ev buf = I[buf_idx][time_idx];
    expr e = slv.ctx.int_val(-1);
    for (auto &[d, dst_pkt_types]: dst_to_pkt_type) {
        expr dst_is_d = slv.ctx.bool_val(false);
        for (int k: dst_pkt_types)
            dst_is_d = dst_is_d || buf[k] > 0;
        e = ite(dst_is_d, slv.ctx.int_val(d), e);
    }
    return e;
}

expr ecmp_val(ev3 &I, SmtSolver &slv, map<int, vector<int> > ecmp_to_pkt_type, int buf_idx, int time_idx) {
    ev buf = I[buf_idx][time_idx];
    expr e = slv.ctx.int_val(-1);
    for (auto &[d, ecmp_pkt_types]: ecmp_to_pkt_type) {
        expr ecmp_is_d = slv.ctx.bool_val(false);
        for (int k: ecmp_pkt_types)
            ecmp_is_d = ecmp_is_d || buf[k] > 0;
        e = ite(ecmp_is_d, slv.ctx.int_val(d), e);
    }
    return e;
}

vector<NamedExp> uniq(ev3 &I, SmtSolver &slv, map<int, vector<int> > dst_to_pkt_type, int num_buffs, int timesteps) {
    vector<NamedExp> v;
    for (int t = 0; t < timesteps; ++t) {
        for (int i = 0; i < num_buffs; ++i) {
            for (int j = 0; j < num_buffs; ++j) {
                if (i == j)
                    continue;
                expr diff = (dst_val(I, slv, dst_to_pkt_type, i, t) !=
                             dst_val(I, slv, dst_to_pkt_type, j, t));
                expr neg = (dst_val(I, slv, dst_to_pkt_type, i, t) == -1
                            && dst_val(I, slv, dst_to_pkt_type, j, t) == -1);
                v.emplace_back(diff || neg, format("unique_t{}_i{}_j{}", t, i, j));
            }
        }
    }
    return v;
}

expr valid(ev3 &I, SmtSolver &slv, int num_buffs, int timesteps) {
    expr res = slv.ctx.bool_val(true);
    for (int i = 0; i < num_buffs; ++i) {
        for (int t = 0; t < timesteps; ++t) {
            res = res && valid_meta(I, slv, i, t);
        }
    }
    return res;
}

expr same(ev3 &I, SmtSolver &slv, map<int, vector<int> > dst_to_pkt_type, int num_buffs, int timesteps) {
    expr res = slv.ctx.bool_val(true);
    for (int i = 0; i < num_buffs; ++i) {
        expr init_val = dst_val(I, slv, dst_to_pkt_type, i, 0);
        res = res && valid_meta(I, slv, i, 0);
        for (int t = 1; t < timesteps; ++t) {
            res = res && ((init_val == dst_val(I, slv, dst_to_pkt_type, i, t)) && valid_meta(I, slv, i, t));
        }
    }
    return res;
}
