#include <bits/stdc++.h>

#include "../utils/vectors.cpp"

#include "node.h"
#include "edge.h"

#ifndef SOFTBODY_EDGE_CC_
#define SOFTBODY_EDGE_CC_

using namespace std;
using namespace utils::vectors;

Edge::Edge() = default;

Edge::Edge(Node node1, Node node2, double spring_coef, double damping_coef, double rest_length) {
    this->node1 = new Node(node1);
    this->node2 = new Node(node2);
    this->spring_coef = spring_coef;
    this->damping_coef = damping_coef;
    this->rest_length = rest_length;
}

// if rest_length not given, set rest_length as the current distance of nodes 1 and 2.
Edge::Edge(Node node1, Node node2, double spring_coef, double damping_coef) {
    this->node1 = new Node(node1);
    this->node2 = new Node(node2);
    this->spring_coef = spring_coef;
    this->damping_coef = damping_coef;

    vector<double> p1 = this->node1->get_position();
    vector<double> p2 = this->node2->get_position();
    this->rest_length = vector_len(vector_sub(p2, p1));
}

Edge::Edge(Node *node1, Node *node2, double spring_coef, double damping_coef) {
    this->node1 = node1;
    this->node2 = node2;
    this->spring_coef = spring_coef;
    this->damping_coef = damping_coef;

    vector<double> p1 = this->node1->get_position();
    vector<double> p2 = this->node2->get_position();
    this->rest_length = vector_len(vector_sub(p2, p1));
}

/*
Edge::~Edge() {
    // this segfaults

    //delete[] this->node1;
    //delete[] this->node2;

    delete this->node1;
    delete this->node2;
}
*/

/*
Edge::Edge(const Edge& e)
    : spring_coef(e.spring_coef),
    damping_coef(e.damping_coef), rest_length(e.rest_length)
{
    node1 = new Node(*e.node1);
    node2 = new Node(*e.node1);
}
*/

void Edge::update_deformation() {
    auto dist_vect = vector_sub(this->node2->get_position(), this->node1->get_position());
    double spring_len = vector_len(dist_vect);
    this->deformation = spring_len - this->rest_length;
}

pair<vector<double>, vector<double>> Edge::calculate_spring_force() {
    update_deformation();
    double magnitude = this->deformation * this->spring_coef;

    vector<double> distance_vect = vector_sub(this->node2->get_position(), this->node1->get_position());
    double distance = vector_len(distance_vect);
    double scale_factor = 0;

    if (distance != 0) {
        scale_factor = magnitude / distance;
    }

    vector<double> force_vect1 = scale_vector(distance_vect, scale_factor);
    vector<double> force_vect2 = scale_vector(force_vect1, -1.0);

    return {force_vect1, force_vect2};
}

pair<vector<double>, vector<double>> Edge::calculate_damping_vectors() {
    // amount of damping varies a lot by time_step (makes sense, minus n-amount of velocity every 1 ms vs every 100ms, 100x difference)

    auto p1 = this->node1->get_position();
    auto p2 = this->node2->get_position();
    auto v1 = this->node1->get_velocity();
    auto v2 = this->node2->get_velocity();

    vector<double> relative_p = vector_sub(p2, p1);
    vector<double> relative_v = vector_sub(v2, v1);

    auto r = project_vector(relative_v, relative_p);
    auto damp_v1 = scale_vector(r, this->damping_coef * 1/2);
    auto damp_v2 = scale_vector(damp_v1, -1.);

    return {damp_v1, damp_v2};
}

void Edge::set_rest_length(double new_rest_length) {
    this->rest_length = new_rest_length;
}

Node* Edge::get_node1() {
    return this->node1;
}

Node* Edge::get_node2() {
    return this->node2;
}

double Edge::get_spring_coef() {
    return this->spring_coef;
}

double Edge::get_damping_coef() {
    return this->damping_coef;
}

double Edge::get_deformation() {
    return this->deformation;
}

double Edge::get_rest_length() {
    return this->rest_length;
}

void Edge::set_id(string id) {
    this->id = id;
}

string Edge::get_edge_id() {
    return this->id;
}

#endif