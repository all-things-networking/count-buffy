//
// Created by Amir Hossein Seyhani on 3/14/25.
//

#include "lib.hpp"

ev &get_buf_vec_at_i(evv const &V, int i) {
    int k = V.size();
    vector<expr> *v = new vector<expr>[k];
    for (int j = 0; j < k; ++j) {
        v->push_back(V[j][i]);
    }
    return *v;
}


vector<int> eval(const ev &v, const model &m) {
    auto *result = new vector<int>[v.size()];
    for (const auto &e: v)
        result->push_back(m.eval(e));
    return *result;
}


vector<vector<int> > eval(const evv &vv, const model &m) {
    auto *result = new vector<vector<int> >[vv.size()];
    for (const auto &v: vv)
        result->push_back(eval(v, m));
    return *result;
}


ostream &operator<<(ostream &os, const vector<int> &v) {
    for (const auto &i: v)
        os << i << " ";
    os << endl;
    return os;
}

ostream &operator<<(ostream &os, const vector<vector<int> > &vv) {
    for (const auto &v: vv)
        os << v << endl;
    return os;
}

void print(const evv &vv, const model &m) {
    for (const auto &v: vv) {
        for (const auto &e: v) {
            // int x = (int) m.eval(e);
            // int x = m.eval(e);
            auto x = m.eval(e);
            cout << x << ", ";

            // if (x.get_sort().is_bool()) {
            //     if (x.bool_value()) {
            //         cout << 1 << ",";
            //     } else {
            //         cout << 0 << ",";
            //     }
            //     cout << x << ",";
            // } else {
            //     cout << x << ", ";
            // }
        }
        cout << endl;
    }
    cout << endl;
}

void print(const ev &v, const model &m) {
    for (const auto &e: v) {
        // int x = (int) m.eval(e);
        // int x = m.eval(e);
        auto x = m.eval(e);
        cout << x << ", ";

        // if (x.get_sort().is_bool()) {
        //     if (x.bool_value()) {
        //         cout << 1 << ",";
        //     } else {
        //         cout << 0 << ",";
        //     }
        //     cout << x << ",";
        // } else {
        //     cout << x << ", ";
        // }
    }
    cout << endl;
}


