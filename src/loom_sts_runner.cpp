//
// Created by Amir Hossein Seyhani on 10/10/25.
//

#include "loom_sts_runner.hpp"

#include <fstream>

#include "utils.hpp"
#include "gen/wl_parser.hpp"
#include <chrono>
#include <filesystem>

const string WORKLOADS_DIR = getenv("BUFFY_WLS_DIR");
const string LOGS_DIR = getenv("BUFFY_LOGS_DIR");

LoomStsRunner::LoomStsRunner(SmtSolver &slv, ev3 &I, ev2 &O, string model, int buff_cap) : slv(slv), model(model), I(I),
    O(O), buff_cap(buff_cap) {
}

vector<NamedExp> LoomStsRunner::base_wl(SmtSolver &slv, const ev3 &ins) {
    vector<NamedExp> res;
    for (int t = 0; t < ins[0].size(); ++t) {
        for (int i = 0; i < ins.size(); ++i) {
            for (int k = 0; k < ins[0][0].size(); ++k) {
                if (i % 3 != k)
                    res.emplace_back(ins[i][t][k] == 0, format("bse_wl: I[{}][{}][{}] == 0", i, t, k));
            }
        }
    }
    for (int t = 0; t < ins[0].size(); ++t) {
        expr s0 = slv.ctx.int_val(0);
        expr s1 = slv.ctx.int_val(0);
        for (int i = 0; i < ins.size(); ++i) {
            if (i % 3 == 0) {
                s0 = s0 + sum(ins[i], t);
            } else {
                s1 = s1 + sum(ins[i], t);
            }
        }
        res.emplace_back(s0 >= t + 1);
        res.emplace_back(s1 >= t + 1, format("I[s1][{}] >= t", t));
    }

    return res;
}

vector<NamedExp> LoomStsRunner::query(SmtSolver &slv, ev2 &out) {
    vector<NamedExp> res;
    ev s = out[0];
    for (int i = 1; i < out.size(); ++i)
        s = s + out[i];
    return {{(s[1] + s[2] - s[0]) > 3, "Query"}};
}

void LoomStsRunner::run() {
    auto bwl = base_wl(slv, I);
    slv.add(bwl);
    string wl_file = format("{}/{}/{}.{}.txt", WORKLOADS_DIR, model, model, buff_cap);
    vector<vector<string> > wls = read_wl_file(wl_file);
    string parent_dir = format("{}/{}", LOGS_DIR, model);
    filesystem::create_directories(parent_dir);
    string out_file_path = format("{}/{}.{}.txt", parent_dir, model, buff_cap);
    ofstream out(out_file_path, ios::out);
    out << "scheduler,buf_size,wl_idx,time_millis,solver_res" << endl;
    for (int i = 0; i < wls.size(); ++i) {
        auto wl = wls[i];
        slv.s.push();
        WorkloadParser parser(I, slv, I.size(), I[0].size());
        string res_stat = wl[0];
        wl.erase(wl.begin());
        parser.parse(wl);

        slv.add(merge(query(slv, O), "not query").negate());
        auto start_t = chrono::high_resolution_clock::now();
        if (res_stat == "SAT")
            slv.check_sat();
        else if (res_stat == "UNSAT")
            slv.check_unsat();
        slv.s.pop();
        auto end_t = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(end_t - start_t);
        cout << "Loom[mem]," << buff_cap << "," << i << "," << duration.count() << "," << res_stat << endl;
        out << "Loom[mem]," << buff_cap << "," << i << "," << duration.count() << "," << res_stat << endl;
    }
}
