//
// Created by Amir Hossein Seyhani on 3/14/25.
//

#ifndef LIB_HPP
#define LIB_HPP


#include <iostream>
#include <sstream>
#include<vector>

#include "named_expr.hpp"
#include"z3++.h"

using namespace std;
using namespace z3;

typedef vector<expr> ev;
typedef vector<ev> ev2;
typedef vector<ev2> ev3;
typedef vector<ev3> ev4;

ev2 &get_buf_vec_at_i(ev3 const &vvv, int i);

ev &get_buf_vec_at_i(ev2 const &vv, int i);

vector<int> eval(const ev2 &v, const model &m);

vector<vector<int> > eval(const ev3 &vv, const model &m);

ostream &operator<<(ostream &os, const vector<int> &v);

ostream &operator<<(ostream &os, const vector<vector<int> > &vv);

stringstream str(const ev &v, const model &m);

stringstream str(const ev2 &vv, const model &m, string sep);

stringstream str(const ev3 &vvv, const model &m);

expr sum(const ev &v);

expr sum(const ev &v, int limit);

expr sum(const ev2 &vv);

expr sum(const ev2 &vv, int limit);

expr operator==(const ev &v, int n);

ev operator+(const ev &l, const ev &r);

ev2 operator+(const ev2 &l, const ev2 &r);

ev operator-(const ev &l, const ev &r);

expr operator==(const ev &l, const ev &r);

expr operator==(const ev2 &l, const ev2 &r);

expr operator<(const ev &l, const int n);

expr operator<(const ev &l, const ev &r);

expr operator<=(const ev &l, int n);

expr operator<=(const ev &l, const ev &r);

expr operator>(const ev &l, int n);

expr operator>=(const ev &l, int n);

expr operator==(const ev &v, const vector<int> &n);

NamedExp merge(const vector<NamedExp> &nes, const string &name);

template<typename T>
void extend(vector<T> &source, const vector<T> &extra) {
    source.insert(source.end(), extra.begin(), extra.end());
}

void extend(vector<NamedExp> &source, const vector<NamedExp> &extra, const string &suffix);

#endif //LIB_HPP
