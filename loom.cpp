#include <iostream>
#include <set>

#include"z3++.h"
#include "src/classifier.hpp"
#include "src/rr_checker.hpp"
#include "src/merger.hpp"
#include "src/prio_sts.hpp"

using namespace std;
using namespace z3;

constexpr int MAX_ENQ = 4;
constexpr int MAX_DEQ = 1;

const int TIME_STEPS = 10;
const int RR_IN_BUFS = 2;
const int PKT_TYPES = 3;

vector<NamedExp> query(SmtSolver &slv, ev2 &out) {
    vector<NamedExp> res;
    ev s = out[0];
    for (int i = 1; i < out.size(); ++i)
        s = s + out[i];
    return {{(s[2] - s[0] - s[1]) >= 3, "Query"}};
}

vector<NamedExp> wl(const ev3 &ins) {
    vector<NamedExp> res;
    for (int i = 0; i < ins.size(); ++i) {
        for (int t = 0; t < ins[i].size(); ++t) {
            res.emplace_back(sum(ins[i][t]) == ins[i][t][i % 3],
                             format("sum(wl[{}][{}]) == wl[{}][{}][{}]", i, t, i, t, i % 3));
            if (i == 3 || i == 8)
                res.emplace_back(ins[i][t] >= 1, format("sum(wl[{}][{}]) >= 1", i, t));
            else
                res.emplace_back(ins[i][t] == 0, format("sum(wl[{}][{}]) == 0", i, t));
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
        // prio->I[1] = classifier->I[1];
    }

    void run() {
        slv.s.push();
        vector rrs = {rr1, rr2, rr3, rr4};
        for (int i = 0; i < rrs.size(); ++i) {
            auto constrs = rrs[i]->base_constrs();
            slv.add(constrs);
            merger->I[i] = rrs[i]->O[0] + rrs[i]->O[1];
        }
        slv.add(merger->base_constrs());
        slv.add(classifier->constrs());
        slv.add(prio->I[1] == classifier->O[0], format("class to prio {}", 1));
        slv.add(prio->I[0] == classifier->O[1], format("class to prio {}", 0));
        slv.add(prio->base_constrs());
        auto base_wl_constrs = wl(I);
        slv.add(base_wl_constrs);
        // slv.add(query(slv, prio->O[0]));
        slv.add(merge(query(slv, prio->O[0]), "not query").negate());
        // slv.add(query(slv, prio->O[0]));
        // auto m = slv.check_sat();
        // print(m);
        // slv.s.pop();
        slv.check_unsat();
        cout << "Buffer Size: " << buffer_size << endl;
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
