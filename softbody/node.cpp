#include <bits/stdc++.h>

#include "../utils/vectors.cpp"

#include "./node.h"

#ifndef SOFTBODY_NODE_CC_
#define SOFTBODY_NODE_CC_

using namespace std;
using namespace utils::vectors;

Node::Node() = default;
Node::Node(vector<double> position, double mass)
{
    this->mass = mass;
    this->acceleration = vector<double>();
    this->velocity = vector<double>();
    this->position = position;
    //init velocity and acceleration as 0 vectors. Determine the number of dimensions from position given as argument
    for (uint i = 0; i < this->position.size(); i++) {
        this->velocity.push_back(0);
        this->acceleration.push_back(0);
    }
}

void Node::set_force(string identifier, vector<double> f_vector) {
    this->forces[identifier] = f_vector;
}

void Node::remove_force(string identifier) {
    this->forces.erase(identifier);
}

vector<double> Node::force_sum() {
    vector<double> sum;
    for (auto pair : this->forces) {
        if (sum.size() == 0) {
            sum = pair.second;
        } else {
            sum = vector_sum(sum, pair.second);
        }
    }

    return sum;
}

void Node::set_velocity(vector<double> velocity) {
    this->velocity = velocity;
}

void Node::set_acceleration(vector<double> acceleration) {
    this->acceleration = acceleration;
}

void Node::set_position(vector<double> position) {
    this->position = position;
}

void Node::update_state(double time_step) {
    auto m = this->mass;
    auto a = this->acceleration;
    auto v = this->velocity;
    auto p = this->position;
    auto f_sum = force_sum();

    vector<double> new_a = scale_vector(f_sum, 1.0/m);
    vector<double> avg_a = scale_vector(vector_sum(a, new_a), 0.5);

    vector<double> new_v = vector_sum(v, scale_vector(avg_a, time_step));
    vector<double> avg_v = scale_vector(vector_sum(v, new_v), 0.5);

    vector<double> new_p = vector_sum(p, scale_vector(avg_v, time_step));

    this->acceleration = new_a;
    this->velocity = new_v;
    this->position = new_p;
}

double Node::get_mass() {
    return this->mass;
}

vector<double> Node::get_acceleration() {
    return this->acceleration;
}

vector<double> Node::get_velocity() {
    return this->velocity;
}

vector<double> Node::get_position() {
    return this->position;
}

vector<double> Node::get_force(string identifier) {
    return this->forces.at(identifier);
}

#endif