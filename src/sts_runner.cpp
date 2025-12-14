//
// Created by Amir Hossein Seyhani on 10/7/25.
//

#include "sts_runner.hpp"

#include <fstream>

#include "gen/wl_parser.hpp"
#include "sts_checker.hpp"
#include "utils.hpp"
#include <chrono>
#include <filesystem>


const string WORKLOADS_DIR = getenv("BUFFY_WLS_DIR");
const string LOGS_DIR = getenv("BUFFY_LOGS_DIR");


StsRunner::StsRunner(STSChecker *sts, string model, int buf_cap) : sts(sts), model(model), buf_cap(buf_cap) {
}

void StsRunner::run(int num_buffers, int timesteps) {
    SmtSolver &slv = sts->slv;
    string wl_file_path = format("{}/{}/{}.{}.txt", WORKLOADS_DIR, model, model, buf_cap);
    cout << "Reading wls from:" << wl_file_path << endl;
    vector<vector<string> > wls = read_wl_file(wl_file_path);
    string parent_dir = format("{}/{}", LOGS_DIR, model);
    filesystem::create_directories(parent_dir);
    string out_file_path = format("{}/{}.{}.txt", parent_dir, model, buf_cap);
    cout << "Writing logs to:" << out_file_path << endl;
    ofstream out(out_file_path, ios::out);
    out << "scheduler, buf_size, wl_idx, time_millis, solver_res" << endl;
    slv.add(sts->base_constrs());
    slv.add(sts->base_wl());
    slv.add(merge(sts->query(), "Query").negate());
    for (int i = 0; i < wls.size(); ++i) {
        WorkloadParser parser(sts->I, slv, num_buffers, timesteps);
        auto wl = wls[i];
        slv.s.push();
        string res_stat = wl[0];
        cout << "WL: " << i + 1 << "/" << wls.size() << " " << res_stat << endl;
        wl.erase(wl.begin());
        parser.parse(wl);

        auto start_t = chrono::high_resolution_clock::now();
        if (res_stat == "SAT")
            slv.check_sat();
        else if (res_stat == "UNSAT") {
            try {
                slv.check_unsat();
            } catch (runtime_error &e) {
                auto m = slv.check_sat();
                cout << "E:" << endl << str(sts->E, m).str();
                cout << "O:" << endl << str(sts->O, m).str();
                cout << "B:" << endl << str(sts->B, m, "\n").str();
                cout << "---------------" << endl;
                exit(1);
            }
        }
        auto end_t = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(end_t - start_t);
        out << model << "," << buf_cap << ", " << i << ", " << duration.count() << ", " << res_stat << endl;
        slv.s.pop();
    }
    out.close();
}
