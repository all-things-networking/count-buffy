//
// Created by Amir Hossein Seyhani on 7/29/25.
//

#include "IntSeq.hpp"


expr IntSeq::create(const string &name) const {
    auto elem_sort = ctx->int_sort();
    auto seq_sort = ctx->seq_sort(elem_sort);
    expr e = ctx->constant(name.c_str(), seq_sort);
    return e;
}

IntSeq::IntSeq(context *ctx) : ctx(ctx) {
}

expr IntSeq::to_expr(Z3_ast ast) const {
    return expr(*ctx, ast);
}

expr IntSeq::length(const expr &s) const {
    Z3_ast ast = Z3_mk_seq_length(*ctx, s);
    return to_expr(ast);
}

expr IntSeq::at(const expr &s, const int i) const {
    Z3_ast ast = Z3_mk_seq_nth(*ctx, s, ctx->int_val(i));
    return to_expr(ast);
}

expr IntSeq::head(const expr &s) const {
    return at(s, 0);
}

expr IntSeq::unit(const expr &x) const {
    Z3_ast x_ast = x;
    const Z3_ast seq = Z3_mk_seq_unit(x.ctx(), x_ast);
    return to_expr(seq);
}

expr IntSeq::push_back(const expr &s, const expr &x) const {
    expr unit_x = unit(x);
    expr new_s = concat(s, unit_x);
    return new_s;
}

expr IntSeq::pop_front(const expr &s) const {
    expr one = ctx->int_val(1);
    expr new_length = length(s) - one;
    Z3_ast extracted_ast = Z3_mk_seq_extract(*ctx, s, one, new_length);
    expr new_s = to_expr(extracted_ast);
    return new_s;
}

expr IntSeq::contains(const expr &s, const expr &x) const {
    Z3_ast ast = Z3_mk_seq_contains(*ctx, s, unit(x));
    return to_expr(ast);
}
