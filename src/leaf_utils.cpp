//
// Created by Amir Hossein Seyhani on 10/8/25.
//

#include "leaf_utils.hpp"

#include <format>
#include <ostream>
#include <string>
#include <vector>
#include <set>

#include "utils.hpp"
#include "gen/leaf_workload_parser.hpp"
#include "gen/wl_parser.hpp"
#include <exprtk.hpp>

using namespace std;

string join(const vector<string> &v, const string &delim = ",") {
    string res;
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) res += delim;
        res += v[i];
    }
    return res;
}

set<int> get_possible_values(vector<string> exprs, set<int> all_vals) {
    typedef exprtk::symbol_table<double> symbol_table_t;
    typedef exprtk::expression<double> expression_t;
    typedef exprtk::parser<double> parser_t;
    double x;
    symbol_table_t table;
    table.add_variable("x", x);
    expression_t expr;
    expr.register_symbol_table(table);

    parser_t parser;
    string conjunction = join(exprs, " and ");
    // cout << "Conjunction: " << conjunction << endl;
    parser.compile(conjunction, expr);
    set<int> possible_values;
    for (auto i: all_vals) {
        x = i;
        if (expr.value())
            possible_values.insert(i);
    }
    return possible_values;
}

void printSet(set<int> &s) {
    for (const auto &x: s)
        cout << x << " ";
    cout << endl;
}

void printMap(map<int, map<string, set<int> > > &m) {
    for (auto &[buf_idx, exprs]: m) {
        cout << "IDX: " << buf_idx << endl;
        cout << "DST:" << endl;
        printSet(exprs["dst"]);
        cout << "ECMP:" << endl;
        printSet(exprs["ecmp"]);
    }
}


map<int, set<int> > parse_meta_constrs(vector<tuple<int, int, string, int> > constrs, set<int> all_vals) {
    map<int, vector<string> > per_buf_map;
    for (auto row: constrs) {
        int time = get<0>(row);
        int buf_idx = get<1>(row);
        string expr = format("{} {} {}", "x", get<2>(row), get<3>(row));
        per_buf_map[buf_idx].push_back(expr);
    }
    map<int, set<int> > values_map;
    for (auto &[buf_idx, exprs]: per_buf_map) {
        auto values = get_possible_values(exprs, all_vals);
        values_map[buf_idx] = values;
    }
    return values_map;
}


map<int, map<string, set<int> > > add_workload(SmtSolver &slv, ev3 &I, int timesteps, map<int, int> pkt_type_to_dst,
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
    auto dst_constrs = parser.dst_constrs;
    auto ecmp_constrs = parser.ecmp_constrs;

    for (int i = 0; i < I.size(); ++i) {
        dst_constrs.emplace_back(0, i, ">=", 0);
        ecmp_constrs.emplace_back(0, i, ">=", 0);
    }

    auto zero_inputs = parser.get_zero_inputs();
    auto dst_values = parse_meta_constrs(dst_constrs, {0, 1, 2, 3, 4, 5});
    auto ecmp_values = parse_meta_constrs(ecmp_constrs, {0, 1});

    map<int, map<string, set<int> > > values_map;
    for (int i = 0; i < I.size(); ++i) {
        if (zero_inputs.contains(i)) {
            values_map[i]["dst"] = {};
            values_map[i]["ecmp"] = {};
        } else {
            values_map[i]["dst"] = dst_values[i];
            values_map[i]["ecmp"] = ecmp_values[i];
        }
    }

    printMap(values_map);
    return values_map;

    // vector<set<int> > result;
    // for (int i = 0; i < I.size(); ++i) {
    //     if (zero_inputs.contains(i))
    //         result.push_back({});
    //     else
    //         result.push_back({4, 5});
    // }
}

set<int> get_used_vals(map<int, map<string, set<int> > > &m, string meta) {
    set<int> used_vals;
    for (auto &[buf_idx, exprs]: m) {
        auto s = exprs[meta];
        for (auto i: s)
            used_vals.insert(i);
    }
    return used_vals;
}

set<int> get_zero_inputs(map<int, map<string, set<int> > > &m) {
    set<int> zero_inputs;
    for (auto &[buf_idx, exprs]: m) {
        auto s = exprs["dst"];
        if (s.empty())
            zero_inputs.insert(buf_idx);
    }
    return zero_inputs;
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
