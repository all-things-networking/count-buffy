//
// Created by Amir Hossein Seyhani on 4/29/25.
//

#include "named_expr.hpp"
#include <format>

NamedExp NamedExp::prefix(const string &prefix) {
    return NamedExp(e, prefix + "_" + name);
}

NamedExp NamedExp::negate() {
    return NamedExp(!e, format("Not ", name));
}
