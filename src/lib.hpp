//
// Created by Amir Hossein Seyhani on 3/14/25.
//

#ifndef LIB_HPP
#define LIB_HPP


#include <iostream>
#include <sstream>
#include<vector>
#include"z3++.h"

using namespace std;
using namespace z3;

typedef vector<expr> ev;
typedef vector<ev> evv;

ev &get_buf_vec_at_i(evv const &V, int i);

vector<int> eval(const ev &v, const model &m);

vector<vector<int> > eval(const evv &vv, const model &m);

ostream &operator<<(ostream &os, const vector<int> &v);

ostream &operator<<(ostream &os, const vector<vector<int> > &vv);

void print(const evv &vv, const model &m);

void print(const ev &v, const model &m);


#endif //LIB_HPP
