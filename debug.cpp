
//expr eq(context &ctx, const ev &v, const vector<int> &vals) {
//    expr res = ctx.bool_val(true);
//    for (int i = 0; i < v.size(); i++)
//        res = res && (v[i] == vals[i]);
//    return res;
//}
//
//expr eq(context &ctx, const ev &v, const vector<bool> &vals) {
//    expr res = ctx.bool_val(true);
//    for (int i = 0; i < v.size(); i++)
//        res = res && implies(v[i], vals[i]) && implies(vals[i], v[i]);
//    return res;
//}
int main(){
    // SmtSolver slv(n);
    // const vector<ev> I = slv.int_vectors(K, "I");
    // const vector<ev> E = slv.int_vectors(K, "E");
    // const vector<ev> D = slv.int_vectors(K, "D");
    // const vector<ev> B = slv.bool_vectors(K, "B");
    // const vector<ev> O = slv.int_vectors(K, "O");
    // const vector<ev> S = slv.int_vectors(K, "S");
    // slv.add(gtz(slv.ctx, I, MAX_ENQ), "I >= 0");
    // slv.add(gtz(slv.ctx, E, MAX_ENQ), "E >= 0");
    // slv.add(gtz(slv.ctx, D, MAX_ENQ), "D >= 0");
    // slv.add(gtz(slv.ctx, O, MAX_DEQ), "O >= 0");
    // slv.add(gtz(slv.ctx, S, C), "S >= 0");
    //
    // slv.add(workload(slv.ctx, I, n), "Workload");
    // slv.add(!query(slv.ctx, B, O, 5, n), "Query");
    // slv.add(out(slv.ctx, B, O, n), "Out");
    // slv.add(trs(slv.ctx, B, n), "Trs");
    // for (int j = 0; j < K; ++j) {
    //     inputs(slv, I, E, D, B, O, S, n, j);
    // }
    // slv.check_unsat();
    // // slv.check_sat();
    // return 0;
    // auto m = slv.check_sat();
    // return 0;

    // const vector i = {0, 2, 2, 0, 0, 0, 1, 0, 0, 0};
    // const vector e = {0, 1, 1, 0, 0, 0, 0, 0, 0, 0};
    // const vector b = {false, true, true, true, true, false, true, true, true, false};
    // const vector s = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    // const vector o = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    // const vector i = {0, 4, 2};
    // const vector e = {0, 1, 1};
    // const vector d = {0, 0, 0};
    // const vector o = {0, 0, 0};
    //

    // slv.add(eq(slv.ctx, I[0], i), "I[0] eq");
    // slv.add(eq(slv.ctx, E[0], e), "E[0] eq");
    // slv.add(eq(slv.ctx, D[0], d), "D[0] eq");
    // slv.add(eq(slv.ctx, B[0], b), "B[0] eq");
    // slv.add(eq(slv.ctx, S[0], s), "S[0] eq");
    // slv.add(eq(slv.ctx, O[0], o), "O[0] eq");
    // bl_size(slv, E, B, S, O, n, 0);
    // enqs(slv, E, B, S, n, 0);
    // drops(slv, D, E, B, S, n, 0);
    // inputs(slv, I, E, D, n, 0);


    // auto m = slv.check_sat();
    // cout << "I0:" << endl;
    // print(I[0], m);
    // cout << "E0:" << endl;
    // print(E[0], m);
    // cout << "D0:" << endl;
    // print(D[0], m);
    // cout << "B0:" << endl;
    // print(B[0], m);
    // cout << "S0:" << endl;
    // print(S[0], m);
    // cout << "O0:" << endl;
    // print(O[0], m);
}
