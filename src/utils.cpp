//
// Created by Amir Hossein Seyhani on 7/7/25.
//

#include "utils.hpp"
#include <fstream>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

vector<vector<string> > read_wl_file() {
    ifstream file("../wl.txt");
    if (!file) {
        cerr << "Error: could not open file" << endl;
    }
    string line;
    vector<vector<string> > sections;
    vector<string> current;

    while (getline(file, line)) {
        if (line.rfind("###", 0) == 0) {
            if (!current.empty()) {
                sections.push_back(current);
                current.clear();
            }
        } else {
            current.push_back(line);
        }
    }

    if (!current.empty()) {
        sections.push_back(current);
    }

    return sections;
}
