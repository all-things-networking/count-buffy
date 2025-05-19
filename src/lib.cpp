//
// Created by Amir Hossein Seyhani on 3/14/25.
//

#include "lib.hpp"

#include "named_expr.hpp"

ev2 &get_buf_vec_at_i(ev3 const &vvv, int i) {
    int k = vvv.size();
    auto v = new vector<vector<expr> >();
    for (int j = 0; j < k; ++j) {
        v->push_back(vvv[j][i]);
    }
    return *v;
}

ev &get_buf_vec_at_i(ev2 const &vv, int i) {
    int k = vv.size();
    auto v = new vector<expr>();
    for (int j = 0; j < k; ++j) {
        v->push_back(vv[j][i]);
    }
    return *v;
}

stringstream str(const ev &v, const model &m) {
    stringstream ss;
    if (v.size() <= 1) {
        ss << m.eval(v[0]);
    } else {
        ss << "<";
        for (const auto &e: v) {
            auto x = m.eval(e);
            ss << x << ",";
        }
        ss << ">";
    }
    return ss;
}

stringstream str(const ev2 &v, const model &m, string sep) {
    stringstream ss;
    for (const auto &e: v)
        ss << str(e, m).str() << sep;
    return ss;
}

stringstream str(const ev3 &vvv, const model &m) {
    stringstream ss;
    for (const auto &vv: vvv) {
        ss << str(vv, m, ", ").str() << endl;
    }
    ss << endl;
    return ss;
}

expr sum(const ev &v) {
    return sum(v, v.size());
}

expr sum(const ev &v, const int limit) {
    expr s = v[0];
    for (int i = 1; i < limit; ++i) {
        s = s + v[i];
    }
    return s;
}

expr sum(const ev2 &vv) {
    return sum(vv, vv.size());
}

expr sum(const ev2 &vv, const int limit) {
    expr s = sum(vv[0]);
    for (int i = 1; i < limit; ++i) {
        s = s + sum(vv[i]);
    }
    return s;
}

expr operator==(const ev &v, const int n) {
    return sum(v) == n;
}

expr operator==(const ev &v, const vector<int> &n) {
    expr result = (v[0] == n[0]);
    for (int i = 1; i < v.size(); ++i) {
        result = result && (v[i] == n[i]);
    }
    return result;
}

ev operator+(const ev &l, const ev &r) {
    ev result;
    for (int i = 0; i < l.size(); ++i) {
        result.push_back(l[i] + r[i]);
    }
    return result;
}

ev operator-(const ev &l, const ev &r) {
    ev result;
    for (int i = 0; i < l.size(); ++i) {
        result.push_back(l[i] - r[i]);
    }
    return result;
}

expr operator==(const ev &l, const ev &r) {
    expr result = (l[0] == r[0]);
    for (int i = 1; i < l.size(); ++i) {
        result = result && (l[i] == r[i]);
    }
    return result;
}

expr operator<(const ev &l, const int n) {
    return sum(l) < n;
}

expr operator<=(const ev &l, const int n) {
    return sum(l) <= n;
}

ev2 operator+(const ev2 &l, const ev2 &r) {
    ev2 result;
    for (int i = 0; i < l.size(); ++i) {
        result.push_back(l[i] + r[i]);
    }
    return result;
}

NamedExp merge(const vector<NamedExp> &nes, const string &name) {
    auto expr = nes[0].e;
    for (int i = 1; i < nes.size(); ++i)
        expr = expr && nes[i].e;
    return {expr, name};
}

expr operator==(const ev2 &l, const ev2 &r) {
    auto res = l[0] == r[0];
    for (int i = 1; i < l.size(); ++i) {
        res = res && (l[i] == r[i]);
    }
    return res;
}

expr operator<=(const ev &l, const ev &r) {
    expr result = (l[0] <= r[0]);
    for (int i = 1; i < l.size(); ++i) {
        result = result && (l[i] <= r[i]);
    }
    return result;
}

expr operator>(const ev &l, const int n) {
    return sum(l) > n;
}

expr operator>=(const ev &l, const int n) {
    return sum(l) >= n;
}

vector<int> &bar() {
    const auto x = new vector<int>(42);
    return *x;
}

void extend(vector<NamedExp> &source, const vector<NamedExp> &extra, const string &suffix) {
    for (const auto &item: extra) {
        NamedExp copy = item;
        copy.name = copy.name + suffix;
        source.push_back(move(copy));
    }
}
