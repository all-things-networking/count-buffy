#include "rate_limiter.hpp"

vector<NamedExp> RateLimiter::workload() {
    vector<NamedExp> rv;
    rv.emplace_back(sum(I[0]) >= 5);
    return rv;
}


RateLimiter::RateLimiter(SmtSolver &slv, const string &var_prefix, int n, int m, int k, int c, int me,
                         int md) : STSChecker(slv,
                                              var_prefix, n, m, k, c, me, md) {
}


vector<NamedExp> RateLimiter::out(const ev &bv, const ev &sv, const ev2 &ov, int t) {
    vector<NamedExp> rv;
    rv.emplace_back(ite(sv[0] == 1, sum(ov[0]) == 1, sum(ov[0]) == 0));
    return rv;
}

vector<NamedExp> RateLimiter::trs(ev const &b, ev const &s, ev const &bp, ev const &sp, int tp) {
    vector<NamedExp> rv;
    rv.emplace_back(ite(bp[0], (sp[0] == (1 - s[0])), (sp[0] == s[0])));
    return rv;
}


vector<NamedExp> RateLimiter::query() {
    vector<NamedExp> rv;
    rv.emplace_back(sum(O[0]) <= 5);
    return rv;
}

vector<NamedExp> RateLimiter::init(const ev &b0, const ev &s0) {
    vector<NamedExp> rv;
    rv.emplace_back(ite(b0[0], s0[0] == 1, s0[0] == 0));
    return rv;
}

void RateLimiter::print(model m) const {
    cout << "Input:" << endl;
    cout << str(I, m).str();
    cout << "Output:" << endl;
    cout << str(O, m).str();
}
