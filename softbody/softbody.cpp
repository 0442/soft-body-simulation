#include <bits/stdc++.h>
#include <math.h>

#include "../utils/vectors.cpp"
#include "../utils/id.cpp"

#include "node.h"
#include "edge.h"
#include "softbody.h"

#ifndef SOFTBODY_SOFTBODY_CC_
#define SOFTBODY_SOFTBODY_CC_

using namespace std;
using namespace utils;
using namespace utils::vectors;

/*
SoftBody::SoftBody() = default;

SoftBody::SoftBody(vector<Node> *nodes, list<Edge> *edges, double edge_deform_at, double edge_deform_coeff, double edge_tear_at)
: this->nodes (nodes), this->edges (edges)
{}
*/

SoftBody::SoftBody(vector<Node> *nodes, list<Edge> *edges, double edge_deform_at, double edge_deform_coeff, double edge_tear_at) {
    this->nodes = nodes;
    this->edges = edges;
    this->edge_deform_at = edge_deform_at;
    this->edge_deform_coef = edge_deform_coef;
    this->edge_tear_at = edge_tear_at;
    set_edge_ids();
}

SoftBody::SoftBody() {
    this->nodes = nodes;
    this->edges = edges;
    this->edge_deform_at = edge_deform_at;
    this->edge_deform_coef = edge_deform_coef;
    this->edge_tear_at = edge_tear_at;
    set_edge_ids();
}

SoftBody::SoftBody(vector<Node> nodes, list<Edge> edges) {
    this->nodes = new vector<Node>(nodes);
    this->edges = new list<Edge>(edges);
    this->edge_deform_at = INF;
    this->edge_deform_coef = INF;
    this->edge_tear_at = INF;
    set_edge_ids();
}

SoftBody::SoftBody(vector<Node> *nodes, list<Edge> *edges) {
    this->nodes = nodes;
    this->edges = edges;
    this->edge_deform_at = INF;
    this->edge_deform_coef = INF;
    this->edge_tear_at = INF;
    set_edge_ids();
}

SoftBody::~SoftBody() {
    delete[] this->nodes;
    delete[] this->edges;
}

void SoftBody::set_external_force(string identifier, vector<double> force_vect) {
    this->external_forces.emplace(identifier, force_vect);
}

void SoftBody::advance_physics(double time_step) {
    list<Edge>::iterator edge_ptr;
    list<Edge>::iterator edge_to_tear = this->edges->end();
    for (edge_ptr = this->edges->begin(); edge_ptr != this->edges->end(); edge_ptr++)
    {
        Node *node1 = edge_ptr->get_node1();
        Node *node2 = edge_ptr->get_node2();

        // tear edge
        if (edge_ptr->get_deformation() > this->edge_tear_at)
        {
            node1->remove_force(edge_ptr->get_edge_id());
            node2->remove_force(edge_ptr->get_edge_id());
            edge_to_tear = edge_ptr;
            continue;
        }

        // deform edge
        if (edge_ptr->get_deformation() > this->edge_deform_at)
            edge_ptr->set_rest_length(edge_ptr->get_rest_length() + edge_ptr->get_deformation());

        // update spring f
        auto f = edge_ptr->calculate_spring_force();
        node1->set_force(edge_ptr->get_edge_id(), f.first);
        node2->set_force(edge_ptr->get_edge_id(), f.second);

        // damping
        auto damp_v = edge_ptr->calculate_damping_vectors();
        vector<double> vel1 = vector_sum(node1->get_velocity(), damp_v.first);
        vector<double> vel2 = vector_sum(node2->get_velocity(), damp_v.second);
        node1->set_velocity(vel1);
        node2->set_velocity(vel2);
    }

    if (edge_to_tear != this->edges->end())
        this->edges->erase(edge_to_tear);

    vector<Node>::iterator n_ptr;
    for (n_ptr = this->nodes->begin(); n_ptr != this->nodes->end(); n_ptr++)
    {
        for (auto f : this->external_forces)
        {
            n_ptr->set_force(f.first, f.second);
        }
        n_ptr->update_state(time_step);
    }
}

void SoftBody::add_velocity(vector<double> v_vect)
{
    vector<Node>::iterator n_ptr;
    for (n_ptr = this->nodes->begin(); n_ptr != this->nodes->end(); n_ptr++)
    {
        vector<double> new_v = vector_sum(n_ptr->get_velocity(), v_vect);
        n_ptr->set_velocity(new_v);
    }
}


void SoftBody::move_relative(vector<double> transform_vect)
{
    vector<Node>::iterator n_ptr;
    for (n_ptr = this->nodes->begin(); n_ptr != this->nodes->end(); n_ptr++)
    {
        n_ptr->set_position(vector_sum(n_ptr->get_position(), transform_vect));
    }
}

void SoftBody::move_absolute(vector<double> top_left_pos)
{
    vector<double> obj_top_left = this->nodes->front().get_position();

    vector<Node>::iterator n_ptr;
    for (n_ptr = this->nodes->begin(); n_ptr != this->nodes->end(); n_ptr++)
    {
        vector<double> pos = n_ptr->get_position();
        obj_top_left[0] = min(pos[0], obj_top_left[0]);
        obj_top_left[1] = min(pos[1], obj_top_left[1]);
    }
    vector<double> transform_vect = vector_sub(obj_top_left, top_left_pos);
    move_relative(transform_vect);
}

void SoftBody::set_edge_ids()
{
    list<Edge>::iterator e_ptr;
    for (e_ptr = this->edges->begin(); e_ptr != this->edges->end(); e_ptr++)
    {
        e_ptr->set_id(utils::a_gen_id());
    }
}

vector<double> SoftBody::get_force(string identifier) {
    return this->external_forces.at(identifier);
}

map<string, vector<double>> SoftBody::get_all_forces() {
    return this->external_forces;
}

vector<Node> *SoftBody::get_nodes() {
    return this->nodes;
}

list<Edge> *SoftBody::get_edges() {
    return this->edges;
}

double SoftBody::get_edge_deform_at() {
    return this->edge_deform_at;
}

double SoftBody::get_edge_tear_at() {
    return this->edge_tear_at;
}

#endif