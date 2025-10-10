//
// Created by Amir Hossein Seyhani on 10/8/25.
//


#include "smt_solver.hpp"
using namespace std;

void add_workload(SmtSolver &slv, ev3& I, int num_spines, int leaf_per_spine, int host_per_leaf, int timesteps);

expr valid_meta(ev3 &I, SmtSolver &slv, int buf_idx, int time_idx);

expr dst_val(ev3 &I, SmtSolver &slv, int buf_idx, int time_idx, int num_buffs);

expr ecmp_val(ev3 &I, SmtSolver &slv, int buf_idx, int time_idx, int num_buffs);

expr uniq(ev3 &I, SmtSolver &slv, int num_buffs, int timesteps);

expr same(ev3 &I, SmtSolver &slv, int num_buffs, int timesteps);
// void add_workload(Am);