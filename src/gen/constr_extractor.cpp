#pragma once

#include "fperfBaseVisitor.h"
#include <iostream>

#include "../smt_solver.hpp"
#include "../lib.hpp"
#include "constr_extractor.hpp"

using namespace std;

expr binop(const expr &left, const std::string &op, const expr &right) {
    if (op == "<")
        return left < right;
    if (op == "<=")
        return left <= right;
    if (op == ">")
        return left > right;
    if (op == ">=")
        return left >= right;
    if (op == "==")
        return left == right;
    if (op == "!=")
        return left != right;
    throw std::invalid_argument("Unknown comparison operator: " + op);
}


using namespace std;

ConstrExtractor::ConstrExtractor(SmtSolver &slv, int n, int m): slv(slv) {
    IT = slv.ivv(n, m, "Workload");
    for (int i = 0; i < IT.size(); ++i) {
        vector<expr> cenqs_i;
        expr cenq = IT[i][0];
        cenqs_i.push_back(cenq);
        for (int j = 1; j < IT[i].size(); ++j) {
            cenq = cenq + IT[i][j];
            cenqs_i.push_back(cenq);
        }
        cenqs.push_back(cenqs_i);
    }
}


any ConstrExtractor::visitCon(fperfParser::ConContext *ctx) {
    auto result = visitChildren(ctx);
    constrs.clear();
    if (metric != "cenq")
        return result;


    assert(begin > 0);
    for (int t = begin; t <= end; ++t) {
        int t_index = t - 1;
        expr s = slv.ctx.int_val(0);
        for (auto i: tmp_ids)
            s = s + cenqs[i][t_index];
        expr e = slv.ctx.bool_val(true);
        if (rhs_linear) {
            e = binop(s, op, slv.ctx.int_val(rhs * t));
        } else {
            e = binop(s, op, slv.ctx.int_val(rhs));
        }
        constrs.push_back(e);
        // cout << "Adding constraint:" << "@[" << t << "]" << metric << "(" << ")" << op << rhs << endl;
    }
    return result;
}

any ConstrExtractor::visitLhs(fperfParser::LhsContext *ctx) {
    return visitChildren(ctx);
}

any ConstrExtractor::visitM(fperfParser::MContext *ctx) {
    metric = ctx->getText();
    return visitChildren(ctx);
}

any ConstrExtractor::visitQ(fperfParser::QContext *ctx) {
    int q = stoi(ctx->INT()->getText());
    tmp_ids.push_back(q);
    return visitChildren(ctx);
}

any ConstrExtractor::visitRhs(fperfParser::RhsContext *ctx) {
    rhs_linear = false;
    if (ctx->INT() && ctx->children.size() == 1)
        rhs = std::stoull(ctx->INT()->getText());
    else if (ctx->getText() == "t") {
        rhs = 1;
        rhs_linear = true;
    } else if (ctx->INT() && ctx->children.size() == 2) {
        rhs = std::stoull(ctx->INT()->getText());
        rhs_linear = true;
    }
    return visitChildren(ctx);
}

any ConstrExtractor::visitInterval(fperfParser::IntervalContext *ctx) {
    begin = stoi(ctx->INT(0)->getText());
    end = stoi(ctx->INT(1)->getText());
    return visitChildren(ctx);
}

any ConstrExtractor::visitSet(fperfParser::SetContext *ctx) {
    for (auto intNode: ctx->INT()) {
        tmp_ids.push_back(stoi(intNode->getText()));
    }
    return visitChildren(ctx);
}

any ConstrExtractor::visitComp_op(fperfParser::Comp_opContext *ctx) {
    op = ctx->getText();
    return visitChildren(ctx);
}
