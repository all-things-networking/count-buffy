#include "src/smt_solver.hpp"

int main() {
    SmtSolver slv;
    auto x = slv.iv(5, "x");
    slv.add(x[0] == 3, "x0");
    slv.add(x[1] == 1, "x1");
    slv.add(x[2] == 1, "x2");
    slv.add(x[3] == 1, "x3");
    slv.add(x[4] == 1, "x4");
    ev capped = slv.capped(x, 5);
    auto m = slv.check_sat();
    for (auto &e: capped)
        cout << m.eval(e) << endl;
}
