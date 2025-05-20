//
// Created by Amir Hossein Seyhani on 4/29/25.
//

#include "named_expr.hpp"

NamedExp NamedExp::prefix(const string &prefix) {
    return {e, prefix + "_" + name};
}

NamedExp NamedExp::negate() {
    return {!e, format("Not ", name)};
}
