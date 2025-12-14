#include "rate_limiter.hpp"
#include "../src/sts_checker.hpp"

constexpr int TIMESTEPS = 10;
constexpr int NUM_BUFS = 1;
constexpr int PKT_TYPES = 1;
constexpr int BUF_CAP = 1000;

int main(const int argc, const char *argv[]) {
    SmtSolver slv;

    STSChecker *sts = new RateLimiter(slv, "rate-limiter", NUM_BUFS, TIMESTEPS, PKT_TYPES, BUF_CAP, 1, 1);

    auto m = sts->check_wl_and_query_sat();
    sts->print(m);

    sts->check_wl_and_not_query_unsat();
}
