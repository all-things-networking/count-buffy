#include <iostream>

#include"z3++.h"
#include "src/classifier.hpp"
#include "src/rr_checker.hpp"
#include "src/merger.hpp"
#include "src/prio_sts.hpp"
#include "src/utils.hpp"
#include "src/gen/wl_parser.hpp"

using namespace std;
using namespace z3;

constexpr int MAX_ENQ = 4;
constexpr int MAX_DEQ = 1;

const int TIME_STEPS = 12;
const int TIME_LIMIT = 4;
const int RR_IN_BUFS = 2;
const int PKT_TYPES = 3;

vector<NamedExp> query(SmtSolver &slv, ev2 &out) {
    vector<NamedExp> res;
    ev s = out[0];
    for (int i = 1; i < out.size(); ++i)
        s = s + out[i];
    return {{(s[1] + s[2] - s[0]) > 3, "Query"}};
}

vector<NamedExp> wl(const ev3 &ins) {
    vector<NamedExp> res;
    for (int i = 0; i < ins.size(); ++i) {
        for (int t = 0; t < TIME_LIMIT; ++t) {
            if (i == 1 || i == 4 || i == 7 || i == 10 || i == 9)
                res.emplace_back(sum(ins[i], t) >= t + 1, format("sum(wl[{}][{}]) >= 1", i, t));
            else
                res.emplace_back(sum(ins[i], t) == 0, format("sum(wl[{}][{}]) == 0", i, t));
        }
    }
    return res;
}

vector<NamedExp> base_wl(SmtSolver &slv, const ev3 &ins) {
    vector<NamedExp> res;
    for (int t = 0; t < TIME_STEPS; ++t) {
        for (int i = 0; i < ins.size(); ++i) {
            for (int k = 0; k < ins[0][0].size(); ++k) {
                if (i % 3 != k)
                    res.emplace_back(ins[i][t][k] == 0, format("bse_wl: I[{}][{}][{}] == 0", i, t, k));
            }
        }
    }
    for (int t = 0; t < TIME_LIMIT; ++t) {
        expr s0 = slv.ctx.int_val(0);
        expr s1 = slv.ctx.int_val(0);
        expr s2 = slv.ctx.int_val(0);
        for (int i = 0; i < ins.size(); ++i) {
            if (i % 3 == 0) {
                s0 = s0 + sum(ins[i], t);
            } else if (i % 3 == 1) {
                s1 = s1 + sum(ins[i], t);
            } else {
                s2 = s2 + sum(ins[i], t);
            }
        }
        // res.emplace_back(s0 >= t + 1, format("I[s0][{}] >= t", t));
        res.emplace_back(s0 >= t + 1, format("I%0 >= t @{}", t));
        res.emplace_back(s1 >= t + 1, format("I%1 >= t @{}", t));
    }

    for (int t = 0; t < TIME_STEPS; ++t) {
        for (int i = 0; i < ins.size(); ++i) {
            if (i % 3 == 2)
                res.emplace_back(sum(ins[i][t]) == 0, format("I[{}][{}] == 0", i, t));
        }
    }

    return res;


    // expr t1 = sum(ins[0][0]) + sum(ins[3][0]) + sum(ins[6][0]) + sum(ins[9][0]);
    // expr t2 = sum(ins[1][0]) + sum(ins[4][0]) + sum(ins[7][0]) + sum(ins[10][0]);
    // res.emplace_back(t1 >= 1, format("I[t1][{}] >= t", 0));
    // res.emplace_back(t2 >= 1, format("I[t2][{}] >= t", 0));
    // for (int t = 1; t < ins[0].size(); ++t) {
    // t1 = t1 + sum(ins[0][t]) + sum(ins[3][t]) + sum(ins[6][t]) + sum(ins[9][t]);
    // t2 = t2 + sum(ins[1][t]) + sum(ins[4][t]) + sum(ins[7][t]) + sum(ins[10][t]);
    // res.emplace_back(t1 >= t + 1, format("I[t1][{}] >= t", t));
    // res.emplace_back(t2 >= t + 1, format("I[t2][{}] >= t", t));
    // }

    // for (int t = 0; t < ins[0].size(); ++t) {
    // for (int i = 0; i < ins.size(); ++i) {
    // if (i % 3 == 2)
    // res.emplace_back(sum(ins[i][t]) == 0, format("I[{}][{}] == 0", i, t));
    // }
    // }
    return res;
}

class Composed {
    ev3 I;
    STSChecker *rr1;
    STSChecker *rr2;
    STSChecker *rr3;
    STSChecker *rr4;
    STSChecker *rrt;
    STSChecker *prio;
    STSChecker *merger;
    Classifier *classifier;
    SmtSolver slv;
    int buffer_size;
    int random_seed;

public:
    Composed(int buffer_size, unsigned int random_seed): slv(random_seed) {
        I = slv.ivvv(4 * PKT_TYPES, TIME_STEPS, PKT_TYPES, "INS");
        slv.add_bound(I, 0, MAX_ENQ);

        rr1 = new RRChecker(slv, "rr1", RR_IN_BUFS, TIME_STEPS, PKT_TYPES, buffer_size, MAX_ENQ, MAX_DEQ);
        rr2 = new RRChecker(slv, "rr2", RR_IN_BUFS, TIME_STEPS, PKT_TYPES, buffer_size, MAX_ENQ, MAX_DEQ);
        rr3 = new RRChecker(slv, "rr3", RR_IN_BUFS, TIME_STEPS, PKT_TYPES, buffer_size, MAX_ENQ, MAX_DEQ);
        rr4 = new RRChecker(slv, "rr4", RR_IN_BUFS, TIME_STEPS, PKT_TYPES, buffer_size, MAX_ENQ, MAX_DEQ);
        vector rrs = {rr1, rr2, rr3, rr4};
        for (int i = 0; i < rrs.size(); ++i) {
            rrs[i]->I[0] = I[i * 3];
            rrs[i]->I[1] = I[i * 3 + 1] + I[i * 3 + 2];
            auto constrs = rrs[i]->base_constrs();
            slv.add(merge(constrs, format("rr{}_base", i)));
        }

        merger = new Merger(slv, "mg", 4, TIME_STEPS, PKT_TYPES, buffer_size, MAX_ENQ, MAX_DEQ);
        for (int i = 0; i < rrs.size(); ++i)
            merger->I[i] = rrs[i]->O[0] + rrs[i]->O[1];
        slv.add(merge(merger->base_constrs(), "merger_base"));

        classifier = new Classifier(slv, "cls", TIME_STEPS, PKT_TYPES, 2);
        classifier->I[0] = merger->O[0] + merger->O[1] + merger->O[2] + merger->O[3];
        classifier->set_out();

        prio = new PrioSTS(slv, "prio", 2, TIME_STEPS, PKT_TYPES, buffer_size, MAX_ENQ, MAX_DEQ);
        prio->I[0] = classifier->O[0];
        prio->I[1] = classifier->O[1];
        slv.add(merge(prio->base_constrs(), "prio_base"));
    }

    void run() {
        auto bwl = base_wl(slv, I);
        slv.add(bwl);
        auto O = prio->O[0] + prio->O[1];

        vector<vector<string> > wls = read_wl_file("../wls/loom.nonmem.txt");
        for (int i = 0; i < wls.size(); ++i) {
            auto wl = wls[i];
            slv.s.push();
            WorkloadParser parser(I, slv, I.size(), I[0].size());
            string res_stat = wl[0];
            wl.erase(wl.begin());
            parser.parse(wl);

            slv.add(merge(query(slv, O), "not query").negate());
            // slv.add(merge(query(slv, O), "query"));
            auto start_t = chrono::high_resolution_clock::now();
            auto m = slv.check_sat();
            print(m);
            slv.s.pop();
            auto end_t = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::milliseconds>(end_t - start_t);
            cout << "loom " << ", " << i << ", " << duration.count() << ", " << res_stat << endl;
            cout << "WL " << i << " Done!" << endl;
        }
    }

    void print(model m) {
        cout << "I:" << endl << str(I, m).str() << endl;
        vector rrs = {rr1, rr2, rr3, rr4};
        for (int i = 0; i < rrs.size(); ++i) {
            auto rr = rrs[i];
            cout << "------------------------------------" << endl;
            cout << "RR" << i << endl;
            cout << str(rr->I, m).str() << endl;
            cout << "*******" << endl;
            cout << str(rr->O, m).str() << endl;
            cout << "------------------------------------" << endl;
        }
        cout << "------------------------------------" << endl;
        cout << "Merger:" << endl;
        cout << "In:" << endl << str(merger->I, m).str() << endl;
        cout << "Out:" << endl << str(merger->O, m).str() << endl;
        cout << "------------------------------------" << endl;
        cout << "Classifier:" << endl;
        cout << "In:" << endl << str(classifier->I, m).str() << endl;
        cout << "Out:" << endl << str(classifier->O, m).str() << endl;
        cout << "------------------------------------" << endl;
        cout << "Priority:" << endl;
        cout << "In:" << endl << str(prio->I, m).str() << endl;
        cout << "Enq:" << endl << str(prio->E, m).str() << endl;
        cout << "Out:" << endl << str(prio->O, m).str() << endl;
        cout << "WE:" << endl;
        cout << str(prio->wnd_enq, m).str();
        cout << "WN:" << endl;
        cout << str(prio->wnd_enq_nxt, m).str();
        cout << "WO:" << endl;
        cout << str(prio->wnd_out, m).str();
        cout << "------------------------------------" << endl;
    }
};

int main(const int argc, const char *argv[]) {
    int buffer_size = stoi(argv[1]);
    int random_seed = stoi(argv[2]);
    Composed c(buffer_size, random_seed);
    c.run();
}
