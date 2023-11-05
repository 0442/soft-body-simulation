#include <bits/stdc++.h>
#include "softbody/softbody.h"

#ifndef SIMULATOR_H_
#define SIMULATOR_H_

using namespace std;

template <typename T>
int sign(T n) { return (0 < n) - (n < 0); }

class Simulator
{
private:
    double bounce_coef;
    double friction_coef;
    vector<SoftBody *> bodies;

public:
    double dsp_w_m = 5;
    double dsp_h_m = 5;

    Simulator();
    Simulator(double bounce_coef, double friction_coef);

    void handle_wall_collisions();
    void __apply_air_resistance();
    void simulate_next_frame(double time_step_s);

    void add_body(SoftBody *body);
    void get_all_nodes(vector<Node *> *out);
    void get_all_edges(vector<Edge *> *out);
};

#endif
