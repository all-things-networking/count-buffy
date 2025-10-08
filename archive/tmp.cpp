#include <iostream>
#include <sstream>
#include<vector>
#include"z3++.h"

using namespace z3;
using namespace std;


void demorgan() {
    std::cout << "de-Morgan example\n";

    context c;

    expr x = c.bool_const("x");
    expr y = c.bool_const("y");
    expr z = c.bool_const("z");
    expr f1 = (implies(!x, y) && implies(x, z));
    expr f2 = ((!x && y) || (x && z));
    expr conjecture = (f1 == f2);
    // expr conjecture = (!(x && y)) == (!x || !y);

    solver s(c);
    s.add(!conjecture);
    std::cout << s << "\n";
    std::cout << s.to_smt2() << "\n";
    auto res = s.check();
    if (res == unsat)
        cout << "Done!" << endl;
    else {
        cout << "SAT!" << endl;
        auto m = s.get_model();
        cout << m.eval(x) << endl;
        cout << m.eval(y) << endl;
        cout << m.eval(z) << endl;
        cout << m.eval(f1) << endl;
        cout << m.eval(f2) << endl;
    }
}

int main() {
    demorgan();
}
