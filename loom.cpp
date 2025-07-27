#include <iostream>
#include <set>

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

const int TIME_STEPS = 1;
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
        for (int t = 0; t < ins[i].size(); ++t) {
            if (i % 3 == 1 || i == 9)
                res.emplace_back(sum(ins[i], t) >= t + 1, format("sum(wl[{}][{}]) >= 1", i, t));
            else
                res.emplace_back(sum(ins[i][t]) == 0, format("sum(wl[{}][{}]) == 0", i, t));
        }
    }
    return res;
}

vector<NamedExp> base_wl(const ev3 &ins) {
    vector<NamedExp> res;
    for (int t = 0; t < ins[0].size(); ++t) {
        for (int i = 0; i < ins.size(); ++i) {
            for (int k = 0; k < ins[0][0].size(); ++k) {
                if (i % 3 != k)
                    res.emplace_back(ins[i][t][k] == 0, format("I[{}][{}][{}] == 0", i, t, k));
            }
        }
    }

    expr t1 = sum(ins[0][0]) + sum(ins[3][0]) + sum(ins[6][0]) + sum(ins[9][0]);
    expr t2 = sum(ins[1][0]) + sum(ins[4][0]) + sum(ins[7][0]) + sum(ins[10][0]);
    res.emplace_back(t1 >= 1, format("I[t1][{}] >= t", 0));
    res.emplace_back(t2 >= 1, format("I[t2][{}] >= t", 0));
    for (int t = 1; t < ins[0].size(); ++t) {
        t1 = t1 + sum(ins[0][t]) + sum(ins[3][t]) + sum(ins[6][t]) + sum(ins[9][t]);
        t2 = t2 + sum(ins[1][t]) + sum(ins[4][t]) + sum(ins[7][t]) + sum(ins[10][t]);
        res.emplace_back(t1 >= t + 1, format("I[t1][{}] >= t", t));
        res.emplace_back(t2 >= t + 1, format("I[t2][{}] >= t", t));
    }

    for (int t = 0; t < ins[0].size(); ++t) {
        for (int i = 0; i < ins.size(); ++i) {
            if (i % 3 == 2)
                res.emplace_back(sum(ins[i][t]) == 0, format("I[{}][{}] == 0", i, t));
        }
    }
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
        }
        merger = new Merger(slv, "mg", 4, TIME_STEPS, PKT_TYPES, buffer_size, MAX_ENQ, MAX_DEQ);;
        classifier = new Classifier(slv, "cls", TIME_STEPS, PKT_TYPES, 2);
        classifier->I[0] = merger->O[0] + merger->O[1] + merger->O[2] + merger->O[3];
        prio = new PrioSTS(slv, "prio", 2, TIME_STEPS, PKT_TYPES, buffer_size, MAX_DEQ, MAX_DEQ);
        this->buffer_size = buffer_size;
        for (int i = 0; i < rrs.size(); ++i) {
            auto constrs = rrs[i]->base_constrs();
            slv.add(constrs);
            merger->I[i] = rrs[i]->O[0] + rrs[i]->O[1];
        }
        slv.add(merger->base_constrs());
        slv.add(classifier->constrs());
        prio->I[0] = classifier->O[0];
        prio->I[1] = classifier->O[1];
        slv.add(prio->base_constrs());
    }

    void run() {
        auto bwl = base_wl(I);
        slv.add(bwl);
        auto O = prio->O[0] + prio->O[1];
        auto mwl = wl(I);
        slv.add(mwl);
        // slv.add(merge(query(slv, O), "not query").negate());
        // slv.add(merge(query(slv, O), "query"));

        auto m = slv.check_sat();
        print(m);
        return;
        vector<vector<string> > wls = read_wl_file("../wls/loom.txt");
        int count = 0;
        for (auto wl: wls) {
            // cout << wl.size() << endl;
            // if (count >= 1)
            // break;

            WorkloadParser parser(I, slv, I.size(), I[0].size());
            string res_stat = wl[0];
            wl.erase(wl.begin());
            try {
                cout << "------------------------------------" << endl;
                cout << ++count << "/" << wls.size() << endl;
                slv.s.push();
                parser.parse(wl);
                if (res_stat == "SAT")
                    slv.check_sat();
                else if (res_stat == "UNSAT")
                    slv.check_unsat();
                // cout << "IT:" << endl;
                // cout << str(I, mod).str();
                slv.s.pop();
            } catch (std::exception &e) {
                cout << "################################### INCOMPATIBLE RESULT ##########################" << endl;
                cout << res_stat << endl;
                slv.s.pop();
            }
        }
    }

    void print(model m) {
        cout << "I:" << endl << str(I, m).str() << endl;
        // cout << "------------------------------------" << endl;
        // cout << str(rr2->O, m).str() << endl;
        // cout << "------------------------------------" << endl;
        // cout << str(rr3->O, m).str() << endl;
        // cout << "------------------------------------" << endl;
        // cout << str(merger->I, m).str() << endl;
        // cout << "------------------------------------" << endl;
        // cout << str(merger->O, m).str() << endl;
        cout << "------------------------------------" << endl;
        cout << str(classifier->I, m).str() << endl;
        cout << "------------------------------------" << endl;
        cout << str(classifier->O, m).str() << endl;
        cout << "------------------------------------" << endl;
        cout << str(prio->I, m).str() << endl;
        cout << "------------------------------------" << endl;
        cout << str(prio->O, m).str() << endl;
        // cout << m.eval(sum(classifier->O[1])) << endl;
        // cout << "------------------------------------" << endl;
        // cout << str(prio->I, m).str() << endl;
        // cout << "------------------------------------" << endl;
        // cout << str(merger->O, m).str() << endl;
        // cout << "O:" << endl << str(prio->O, m).str() << endl;
        // for (auto rr: {rr1, rr2, rr3, rr4}) {
        //     cout << rr->var_prefix << endl;
        //     cout << "E:" << endl << str(rr->E, m).str();
        //     cout << "O:" << endl << str(rr->O, m).str();
        //     cout << "---------------" << endl;
        // }
        // cout << merger->var_prefix << endl;
        // cout << "E:" << endl << str(merger->E, m).str();
        // cout << "O:" << endl << str(O, m, ",").str();
        // cout << "---------------" << endl;
    }
};

int main(const int argc, const char *argv[]) {
    int buffer_size = std::stoi(argv[1]);
    int random_seed = std::stoi(argv[2]);
    Composed c(buffer_size, random_seed);
    c.run();
}
