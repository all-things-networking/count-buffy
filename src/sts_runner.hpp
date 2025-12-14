//
// Created by Amir Hossein Seyhani on 10/7/25.
//

#ifndef STS_RUNNER_HPP
#define STS_RUNNER_HPP
#include <string>

class STSChecker;
using namespace std;

class StsRunner {
private:
    int buf_cap;
    string model;
    STSChecker* sts;
public:
    StsRunner(STSChecker *sts, string model, int buf_cap);

    void print_stats();

    void run(int num_buffers, int timesteps);
};


#endif //STS_RUNNER_HPP
