#include <bits/stdc++.h>
#include "node.h"
#include "edge.h"

#ifndef SOFTBODY_SOFTBODY_H_
#define SOFTBODY_SOFTBODY_H_

#define INF numeric_limits<double>::infinity();
using namespace std;

class SoftBody
{
private:
    vector<Node> *nodes;
    list<Edge> *edges;
    double edge_deform_at;
    double edge_deform_coef;
    double edge_tear_at;
    map<string, vector<double>> external_forces;

public:
    SoftBody();
    SoftBody(vector<Node> *nodes, list<Edge> *edges, double edge_deform_at, double edge_deform_coeff, double edge_tear_at);
    SoftBody(vector<Node> nodes, list<Edge> edges);
    SoftBody(vector<Node> *nodes, list<Edge> *edges);
    ~SoftBody();

    void set_external_force(string identifier, vector<double> force_vect);

    void advance_physics(double time_step);

    void add_velocity(vector<double> v_vect);
    void move_relative(vector<double> transform_vect);
    void move_absolute(vector<double> top_left_pos);

    void set_edge_ids();

    vector<double> get_force(string identifier);
    map<string, vector<double>> get_all_forces();
    vector<Node> *get_nodes();
    list<Edge> *get_edges();
    double get_edge_deform_at();
    double get_edge_tear_at();
};

#endif