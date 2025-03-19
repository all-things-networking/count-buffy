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
typedef vector<vector<expr> > evv;
typedef vector<evv> evvv;

evv &get_buf_vec_at_i(evvv const &vvv, int i);

ev &get_buf_vec_at_i(evv const &vv, int i);

vector<int> eval(const evv &v, const model &m);

vector<vector<int> > eval(const evvv &vv, const model &m);

ostream &operator<<(ostream &os, const vector<int> &v);

ostream &operator<<(ostream &os, const vector<vector<int> > &vv);

stringstream str(const ev &v, const model &m);

stringstream str(const evv &vv, const model &m, string sep);

stringstream str(const evvv &vvv, const model &m);

expr sum(const ev &v);

expr sum(const ev &v, int limit);

expr sum(const evv &vv);

expr sum(const evv &vv, int limit);

expr operator==(const ev &v, int n);

ev operator+(const ev &l, const ev &r);

ev operator-(const ev &l, const ev &r);

expr operator==(const ev &l, const ev &r);

expr operator<=(const ev &l, const int n);

expr operator>(const ev &l, const int n);


#endif //LIB_HPP
