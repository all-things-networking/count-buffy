#include "leaf_sts.hpp"

#include "prio_sts.hpp"

LeafSts::LeafSts(SmtSolver &slv, const string &var_prefix, int num_ports, int time_steps, int c, int me,
                 int md): STSChecker(slv, var_prefix, num_ports * num_ports, time_steps, 1, c, me, md),
                          num_ports(num_ports) {
}

vector<NamedExp> LeafSts::workload() {
    vector<NamedExp> res;
    return res;
}

vector<NamedExp> LeafSts::out(const ev &bv, const ev &sv, const ev2 &ov, int t) {
    expr res = slv.ctx.bool_val(true);
    for (int src = 0; src < num_ports; ++src) {
        for (int dst = 0; dst < num_ports; ++dst) {
            int idx = src * num_ports + dst;
            expr turn = sv[dst];
            res = res && ite(bv[idx] && turn == slv.ctx.int_val(src), ov[idx] == 1, ov[idx] == 0);
        }
    }
    return {res};
}

expr LeafSts::rr(ev const &backlog, expr &prev_turn) {
    int count = backlog.size();
    expr nxt_turn = slv.ctx.int_val(0);
    for (int i = 0; i < count; ++i) {
        expr x = slv.ctx.int_val(i);
        for (int j = 1; j < count; ++j) {
            const int l = (i - j + count) % count;
            x = ite(backlog[l], slv.ctx.int_val(l), x);
        }
        nxt_turn = ite(prev_turn == i, x, nxt_turn);
    }
    return nxt_turn;
}

vector<NamedExp> LeafSts::trs(ev const &b, ev const &s, ev const &bp, ev const &sp, int tp) {
    vector<NamedExp> v;
    for (int out_idx = 0; out_idx < num_ports; ++out_idx) {
        expr prev_turn = s[out_idx];
        ev backlogs_of_out_i = get_voq_of_out_i(b, out_idx);
        expr nxt_turn = rr(backlogs_of_out_i, prev_turn);
        v.emplace_back(sp[out_idx] == nxt_turn);
    }
    return v;
}


vector<NamedExp> LeafSts::query(const int p) {
    expr res = slv.ctx.bool_val(false);
    return {res};
}

vector<NamedExp> LeafSts::init(const ev &b0, const ev &s0) {
    expr res = slv.ctx.bool_val(true);
    for (int i = 0; i < num_ports; ++i)
        res = res && (s0[i] == 0);

    return {res};
}

template<typename V>
V LeafSts::get_voq_of_out_i(const V &all_ev, const int i) {
    V ev;
    int dst = i;
    for (int src = 0; src < num_ports; ++src) {
        int idx = src * num_ports + dst;
        ev.push_back(all_ev[idx]);
    }
    return ev;
}

void LeafSts::print(model mod) const {
    for (int src = 0; src < num_ports; ++src) {
        cout << "------------------------" << endl;
        cout << "In[" << src << "]: " << endl;
        for (int dst = 0; dst < num_ports; ++dst) {
            auto idx = src * num_ports + dst;
            cout << "TO[" << dst << "]: " << str(I[idx], mod, ",").str() << endl;
        }
    }
    cout << endl;
    for (int dst = 0; dst < num_ports; ++dst) {
        cout << "------------------------" << endl;
        cout << "Out[" << dst << "]: " << endl;
        for (int src = 0; src < num_ports; ++src) {
            auto idx = src * num_ports + dst;
            cout << "FROM[" << src << "]: " << str(O[idx], mod, ",").str() << endl;
        }
    }
    cout << endl;
    for (int i = 0; i < num_ports; ++i) {
        cout << "------------------------" << endl;
        cout << "Turn Dst[" << i << "]" << endl;
        cout << str(get_state()[i], mod).str() << endl;
    }
    // for (int i = 0; i < num_ports; ++i) {
    // cout << "------------------------" << endl;
    // cout << "M = " << i << endl;
    // cout << str(I[i], mod, ",").str() << endl;
    // for (int j = 0; j < k; ++j) {
    // cout << str(I[i * k + j], mod, ",").str() << endl;
    // }
    // }
}
