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
public:
    SmtSolver &slv;
    vector<int> tmp_ids;
    vector<int> max_t_with_zero_cenq;
    int begin;
    int end;
    string metric;
    string op;
    int rhs;
    bool rhs_linear;
    ev2 cenqs;
    ev2 aipgs;
    int num_buffs;
    int timesteps;
    map<int, vector<int> > dst_to_pkt_type;
    map<int, vector<int> > ecmp_to_pkt_type;

public:
    vector<NamedExp> constrs;
    ev2 IT;
    ev3 I;
    ev2 dsts;
    ev2 ecmps;

    ConstrExtractor(SmtSolver &slv, ev3 &I, int timesteps,
                    map<int, vector<int> > dst_to_pkt_type,
                    map<int, vector<int> > ecmp_to_pkt_type);

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
