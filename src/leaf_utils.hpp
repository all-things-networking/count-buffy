//
// Created by Amir Hossein Seyhani on 10/8/25.
//


#include <map>

#include "smt_solver.hpp"
using namespace std;

set<int> add_workload(SmtSolver &slv, ev3 &I, int timesteps, map<int, int> pkt_type_to_dst, map<int, int> pkt_type_to_ecmp);

expr valid_meta(ev3 &I, SmtSolver &slv, int buf_idx, int time_idx);

expr dst_val(ev3 &I, SmtSolver &slv, map<int, vector<int> > dst_to_pkt_type, int buf_idx, int time_idx);

expr ecmp_val(ev3 &I, SmtSolver &slv, map<int, vector<int> > ecmp_to_pkt_type, int buf_idx, int time_idx);

expr valid(ev3 &I, SmtSolver &slv, int num_buffs, int timesteps);

vector<NamedExp> uniq(ev3 &I, SmtSolver &slv, map<int, vector<int> > dst_to_pkt_type, int num_buffs, int timesteps);

expr same(ev3 &I, SmtSolver &slv, map<int, vector<int> > dst_to_pkt_type, int num_buffs, int timesteps);

// void add_workload(Am);
