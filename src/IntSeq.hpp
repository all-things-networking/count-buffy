//
// Created by Amir Hossein Seyhani on 7/29/25.
//

#ifndef INT_SEQ_HPP
#define INT_SEQ_HPP
#include <z3++.h>

using namespace z3;
using namespace std;

class IntSeq {
public:
    IntSeq(context *ctx);

    expr create(const string &name) const;

    expr to_expr(Z3_ast ast) const;

    expr length(const expr& s) const;

    expr at(const expr& s, const int i) const;

    expr head(const expr& s) const;

    expr unit(const expr &x) const;

    expr push_back(const expr &s, const expr &x) const;

    expr push_back(const expr &s, int i) const;

    expr pop_front(const expr &s) const;

    expr contains(const expr &s, const expr &x) const;

    context* ctx;
};


#endif //INT_SEQ_HPP
