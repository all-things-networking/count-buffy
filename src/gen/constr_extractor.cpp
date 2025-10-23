#pragma once

#include "fperfBaseVisitor.h"
#include <iostream>

#include "../smt_solver.hpp"
#include "../lib.hpp"
#include "constr_extractor.hpp"

#include "../utils.hpp"
#include "../leaf_utils.hpp"

using namespace std;

ConstrExtractor::ConstrExtractor(SmtSolver &slv, ev3 &I, int timesteps, map<int, vector<int> > dst_to_pkt_type,
                                 map<int, vector<int> > ecmp_to_pkt_type): slv(slv), timesteps(timesteps), I(I),
                                                                           dst_to_pkt_type(dst_to_pkt_type),
                                                                           ecmp_to_pkt_type(ecmp_to_pkt_type) {
    IT = slv.ivv(I.size(), timesteps, "Workload");
    for (int i = 0; i < IT.size(); ++i) {
        auto cenqs_i = get_cenqs_for_buff(IT[i]);
        cenqs.push_back(cenqs_i);
        auto aipgs_i = get_aipgs_for_buf(IT[i]);
        aipgs.push_back(aipgs_i);
    }
    for (int i = 0; i < I.size(); ++i) {
        ev buf_ev;
        for (int t = 0; t < timesteps; ++t) {
            buf_ev.push_back(dst_val(I, slv, dst_to_pkt_type, i, t));
        }
        dsts.push_back(buf_ev);
    }

    for (int i = 0; i < I.size(); ++i) {
        ev buf_ev;
        for (int t = 0; t < timesteps; ++t) {
            buf_ev.push_back(ecmp_val(I, slv, ecmp_to_pkt_type, i, t));
        }
        ecmps.push_back(buf_ev);
    }
    num_buffs = IT.size();
}

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


void ConstrExtractor::print(model m) const {
    cout << "IT:" << endl;
    cout << str(IT, m, "\n").str();
    // cout << "ECMP:" << endl;
    // cout << str(ECMP, m, "\n").str();
    // cout << "DST:" << endl;
    // cout << str(DST, m, "\n").str();
}


ev ConstrExtractor::get_cenqs_for_buff(ev it) {
    ev cenqs_i;
    expr cenq = it[0];
    cenqs_i.push_back(cenq);
    for (int j = 1; j < it.size(); ++j) {
        cenq = cenq + it[j];
        cenqs_i.push_back(cenq);
    }
    return cenqs_i;
}

ev ConstrExtractor::get_aipgs_for_buf(ev it) const {
    int total_time = it.size();
    ev aipgs_i;
    aipgs_i.push_back(slv.ctx.int_val(0));
    for (unsigned int t1 = 1; t1 < total_time; t1++) {
        expr val_expr = slv.ctx.int_val(total_time);

        for (unsigned int t2 = total_time - 1; t2 > t1; t2--) {
            val_expr = ite(it[t2] > 0, slv.ctx.int_val(t2 - t1), val_expr);
        }

        for (unsigned int t2 = 0; t2 < t1; t2++) {
            val_expr = ite(it[t2] > 0, slv.ctx.int_val(t1 - t2), val_expr);
        }
        auto expr = ite(it[t1] > 1,
                        slv.ctx.int_val(0),
                        ite(it[t1] <= 0, aipgs_i[t1 - 1], val_expr));
        aipgs_i.push_back(expr);
    }
    return aipgs_i;
}

void ConstrExtractor::parse_cenq() {
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
        string constr_name = format("CENQ[{},{}] {} {}", join_vec(tmp_ids), t_index, op, rhs);
        constrs.emplace_back(e, constr_name);
        // cout << "Adding constraint:" << "@[" << t << "]" << metric << "(" << ")" << op << rhs << endl;
    }
}

void ConstrExtractor::parse_aipg() {
    assert(begin > 0);
    for (int t = begin; t <= end; ++t) {
        int t_index = t - 1;
        assert(tmp_ids.size() == 1);
        int i = tmp_ids[0];
        expr s = aipgs[i][t_index];
        expr e = slv.ctx.bool_val(true);
        if (rhs_linear) {
            e = binop(s, op, slv.ctx.int_val(rhs * t));
        } else {
            e = binop(s, op, slv.ctx.int_val(rhs));
        }
        constrs.push_back(e);
        // cout << "Adding constraint:" << "@[" << t << "]" << metric << "(" << ")" << op << rhs << endl;
    }
}

void ConstrExtractor::parse_dst() {
    assert(begin > 0);
    for (int t = begin; t <= end; ++t) {
        int t_index = t - 1;
        assert(tmp_ids.size() == 1);
        assert(!rhs_linear);
        int buf_index = tmp_ids[0];
        expr dst_value = dst_val(I, slv, dst_to_pkt_type, buf_index, t_index);
        expr e = binop(dst_value, op, slv.ctx.int_val(rhs));
        constrs.emplace_back(valid_meta(I, slv, buf_index, t_index), format("DST[{}][{}] valid", buf_index, t_index));
        constrs.emplace_back(e, format("DST[{}][{}] {} {}", buf_index, t_index, op, rhs));
    }
}

void ConstrExtractor::parse_ecmp() {
    assert(begin > 0);
    for (int t = begin; t <= end; ++t) {
        int t_index = t - 1;
        assert(tmp_ids.size() == 1);
        assert(!rhs_linear);
        int buf_index = tmp_ids[0];
        expr ecmp_value = ecmp_val(I, slv, ecmp_to_pkt_type, buf_index, t_index);
        expr e = binop(ecmp_value, op, slv.ctx.int_val(rhs));
        // constrs.emplace_back(implies(valid_meta(I, slv, buf_index, t_index), e),
        // format("ECMP[{}]@[{}] {} {}", buf_index, t_index, op, rhs));
        // constrs.emplace_back(e, format("ECMP[{}]@[{}] {} {}", buf_index, t_index, op, rhs));
        constrs.emplace_back(NamedExp(e).prefix(format("ECMP[{}]@[{}] {} {}", buf_index, t_index, op, rhs)));
    }
}


any ConstrExtractor::visitCon(fperfParser::ConContext *ctx) {
    auto result = visitChildren(ctx);
    constrs.clear();
    if (metric == "cenq") {
        parse_cenq();
        return result;
    }
    if (metric == "aipg") {
        parse_aipg();
        return result;
    }

    if (metric == "dst") {
        parse_dst();
        return result;
    }

    if (metric == "ecmp") {
        parse_ecmp();
        return result;
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

any ConstrExtractor::visitMm(fperfParser::MmContext *ctx) {
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
