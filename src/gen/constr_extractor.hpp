#pragma once

#include "fperfBaseVisitor.h"
#include <iostream>

#include "../smt_solver.hpp"
#include "../lib.hpp"

using namespace std;

const std::map<int, int> id_map = {{1, 2}, {2, 3}};

class Data {
public:
    int t;
    string m;
    vector<int> ids;
};

expr binop(const expr &left, const std::string &op, const expr &right);


class ConstrExtractor : public fperfBaseVisitor {
    SmtSolver &slv;
    std::vector<int> tmp_ids;
    int begin;
    int end;
    std::string metric;
    std::string op;
    int rhs;
    bool rhs_linear;
    ev2 cenqs;
    ev2 aipgs;

public:
    vector<expr> constrs;
    ev2 IT;

    ConstrExtractor(SmtSolver &slv, int n, int m);

    ev get_cenqs_for_buff(ev it);

    ev get_aipgs_for_buf(ev it) const;

    void parse_cenq();

    void parse_aipg();

    std::any visitCon(fperfParser::ConContext *ctx) override;

    std::any visitLhs(fperfParser::LhsContext *ctx) override;

    std::any visitM(fperfParser::MContext *ctx) override;

    std::any visitQ(fperfParser::QContext *ctx) override;

    std::any visitRhs(fperfParser::RhsContext *ctx) override;

    std::any visitInterval(fperfParser::IntervalContext *ctx) override;

    std::any visitSet(fperfParser::SetContext *ctx) override;

    std::any visitComp_op(fperfParser::Comp_opContext *ctx) override;
};
