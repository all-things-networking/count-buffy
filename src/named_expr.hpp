//
// Created by Amir Hossein Seyhani on 4/29/25.
//

#ifndef NAMED_EXPR_HPP
#define NAMED_EXPR_HPP
#include <z3++.h>

#include <utility>

using namespace z3;
using namespace std;

class NamedExp {
public:
    expr e;
    string name;


    NamedExp(expr e, string name): e(std::move(e)), name(std::move(name)) {
    }

    NamedExp(expr e): e(std::move(e)), name(e.to_string()) {
    }

    NamedExp prefix(const string &prefix);

    NamedExp negate();
};


#endif //NAMED_EXPR_HPP
