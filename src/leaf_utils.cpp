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

void add_workload() {

}