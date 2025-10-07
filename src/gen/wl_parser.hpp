//
// Created by Amir Hossein Seyhani on 7/9/25.
//

#ifndef WL_PARSER_HPP
#define WL_PARSER_HPP
#include <vector>
#include "../named_expr.hpp"
#include "../smt_solver.hpp"

using namespace std;


class WorkloadParser {
public:
    WorkloadParser(ev3 &I, SmtSolver &slv, int n, int m);

    expr parse(string prefix, string wl_line);

    void parse(vector<string> wl);

    void parse_file();

private:
    int n;
    int m;
    ev3 I;
    SmtSolver &slv;
};


#endif //WL_PARSER_HPP
