#pragma once

#include "fperfBaseVisitor.h"

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

expr binop(const expr &left, const string &op, const expr &right);


class ConstrExtractor : public fperfBaseVisitor {
    SmtSolver &slv;
    vector<int> tmp_ids;
    int begin;
    int end;
    string metric;
    string op;
    int rhs;
    bool rhs_linear;
    ev2 cenqs;
    ev2 aipgs;

public:
    vector<expr> constrs;
    ev2 IT;
    map<int, map<int, int> > DST;
    map<int, map<int, int> > ECMP;

    ConstrExtractor(SmtSolver &slv, int n, int m);

    void print(model m) const;

    ev get_cenqs_for_buff(ev it);

    ev get_aipgs_for_buf(ev it) const;

    void parse_cenq();

    void parse_aipg();

    void parse_dst();

    void parse_ecmp();

    any visitCon(fperfParser::ConContext *ctx) override;

    any visitLhs(fperfParser::LhsContext *ctx) override;

    any visitM(fperfParser::MContext *ctx) override;

    any visitMm(fperfParser::MmContext *ctx) override;

    any visitQ(fperfParser::QContext *ctx) override;

    any visitRhs(fperfParser::RhsContext *ctx) override;

    any visitInterval(fperfParser::IntervalContext *ctx) override;

    any visitSet(fperfParser::SetContext *ctx) override;

    any visitComp_op(fperfParser::Comp_opContext *ctx) override;
};
