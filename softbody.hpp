#include <bits/stdc++.h>
#include <math.h>
#include "./utils/vector_operations.hpp"
#include "./utils/id.hpp"

#ifndef SOFTBODY_H
#define SOFTBODY_H


using namespace std;

class Node {
    private:
        double mass; // allows negative mass
        vector<double> acceleration;
        vector<double> velocity;
        vector<double> position;
        unordered_map<string, vector<double>> forces;

    public:
        Node() = default;
        Node(vector<double>position, double mass)
        {
            this->mass = mass;
            this->position = position;
            this->velocity = vector<double>();
            this->acceleration = vector<double>();
            //determine the simulated dimensions from position given as argument, apply to vel and accel
            for (int i = 0; i < this->position.size(); i++) {
                this->velocity.push_back(0);
                this->acceleration.push_back(0);
            }
        }

        void set_force(string identifier, vector<double> f_vector) {
            this->forces[identifier] = f_vector;
        }

        void remove_force(string identifier) {
            this->forces.erase(identifier);
        }

        void set_velocity(vector<double> velocity) {
            this->velocity = velocity;
        }

        void set_acceleration(vector<double> acceleration) {
            this->acceleration = acceleration;
        }

        void set_position(vector<double> position) {
            this->position = position;
        }

        vector<double> force_sum() {
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

        void update_state(double time_step) {
            auto m = this->mass;
            //auto a = this->acceleration;
            auto v = this->velocity;
            auto p = this->position;
            auto f_sum = force_sum();

            vector<double> new_a = scale_vector(f_sum, 1.0/m);
            //vector<double> avg_a = scale_vector(vector_sum(a, new_a), 0.5);

            vector<double> new_v = vector_sum(v, scale_vector(new_a, time_step));
            vector<double> avg_v = scale_vector(vector_sum(v, new_v), 0.5);

            vector<double> new_p = vector_sum(p, scale_vector(avg_v, time_step));

            this->acceleration = new_a;
            this->velocity = new_v;
            this->position = new_p;
        }

        double get_mass() {
            return this->mass;
        }
        vector<double> get_acceleration() {
            return this->acceleration;
        }
        vector<double> get_velocity() {
            return this->velocity;
        }
        vector<double> get_position() {
            return this->position;
        }
        vector<double> get_force(string identifier) {
            return this->forces.at(identifier);
        }
};

class Edge {
    private:
        Node* node1;
        Node* node2;
        double spring_coef;
        double damping_coef;
        double rest_length;
        double deformation = 0;
        string id;

    public:
        Edge() = default;
        Edge(Node* node1, Node* node2, double spring_coef, double damping_coef, double rest_length) {
            this->node1 = node1;
            this->node2 = node2;
            this->spring_coef = spring_coef;
            this->damping_coef = damping_coef;
            this->rest_length = rest_length;
        }
        // if rest_length not given, set rest_length as the current distance of nodes 1 and 2.
        Edge(Node* node1, Node* node2, double spring_coef, double damping_coef) {
            this->node1 = node1;
            this->node2 = node2;
            this->spring_coef = spring_coef;
            this->damping_coef = damping_coef;

            vector<double> p1 = node1->get_position();
            vector<double> p2 = node2->get_position();
            this->rest_length = vector_len(vector_sub(p2, p1));
        }

        void update_deformation() {
            auto dist_vect = vector_sub(this->node2->get_position(), this->node1->get_position());
            double spring_len = vector_len(dist_vect);
            this->deformation = spring_len - this->rest_length;
        }

        pair<vector<double>, vector<double>> calculate_spring_force() {
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

        pair<vector<double>, vector<double>> calculate_damping_vectors() {
            auto p1 = this->node1->get_position();
            auto p2 = this->node2->get_position();
            auto v1 = this->node1->get_velocity();
            auto v2 = this->node2->get_velocity();

            vector<double> relative_p = vector_sub(p2, p1);
            vector<double> relative_v = vector_sub(v2, v1);

            auto r = project_vector(relative_v, relative_p);
            auto damp_v1 = scale_vector(r, this->damping_coef);
            auto damp_v2 = scale_vector(damp_v1, -1.);

            return {damp_v1, damp_v2};
        }

        void set_rest_length(double new_rest_length) {
            this->rest_length = new_rest_length;
        }

        Node* get_node1() {
            return this->node1;
        }
        Node* get_node2() {
            return this->node2;
        }
        double get_spring_coef() { return this->spring_coef; }
        double get_damping_coef() { return this->damping_coef; }
        double get_deformation() { return this->deformation; }
        double get_rest_length() { return this->rest_length; }

        // id used for distinguishing between other edges, e.g. when there are multiple spring/edge forces acting on one node.
        void set_id(string id) {
            this->id = id;
        }
        string get_edge_id() {
            return this->id;
        }
};

class SoftBody
{
private:
    const double INF = numeric_limits<double>::infinity();
    vector<Node> *nodes;
    list<Edge> *edges;
    double edge_deform_at;
    double edge_deform_coef;
    double edge_tear_at;
    map<string, vector<double>> external_forces;

public:
    SoftBody() = default;
    SoftBody(vector<Node> *nodes, list<Edge> *edges, double edge_deform_at, double edge_deform_coeff, double edge_tear_at)
    {
        this->nodes = nodes;
        this->edges = edges;
        this->edge_deform_at = edge_deform_at;
        this->edge_deform_coef = edge_deform_coef;
        this->edge_tear_at = edge_tear_at;
        set_edge_ids();
    }
    SoftBody(vector<Node> *nodes, list<Edge> *edges)
    {
        this->nodes = nodes;
        this->edges = edges;
        this->edge_deform_at = INF;
        this->edge_deform_coef = INF;
        this->edge_tear_at = INF;
        set_edge_ids();
    }

    void set_external_force(string identifier, vector<double> force_vect)
    {
        auto r = this->external_forces.emplace(identifier, force_vect);
    }

    void advance_physics(double time_step)
    {
        list<Edge>::iterator edge_ptr;
        for (edge_ptr = this->edges->begin(); edge_ptr != this->edges->end(); edge_ptr++)
        {
            Node *node1 = edge_ptr->get_node1();
            Node *node2 = edge_ptr->get_node2();

            // tear edge
            if (edge_ptr->get_deformation() > this->edge_tear_at)
            {
                node1->remove_force(edge_ptr->get_edge_id());
                node2->remove_force(edge_ptr->get_edge_id());
                this->edges->remove(*edge_ptr);
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

    void add_velocity(vector<double> v_vect)
    {
        vector<Node>::iterator n_ptr;
        for (n_ptr = this->nodes->begin(); n_ptr != this->nodes->end(); n_ptr++)
        {
            vector<double> new_v = vector_sum(n_ptr->get_velocity(), v_vect);
            n_ptr->set_velocity(new_v);
        }
    }

    void move_relative(vector<double> transform_vect)
    {
        vector<Node>::iterator n_ptr;
        for (n_ptr = this->nodes->begin(); n_ptr != this->nodes->end(); n_ptr++)
        {
            n_ptr->set_position(vector_sum(n_ptr->get_position(), transform_vect));
        }
    }

    void move_absolute(vector<double> top_left_pos)
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

    void set_edge_ids()
    {
        list<Edge>::iterator e_ptr;
        for (e_ptr = this->edges->begin(); e_ptr != this->edges->end(); e_ptr++)
        {
            e_ptr->set_id(gen_id());
        }
    }

    vector<double> get_force(string identifier) {
        return this->external_forces.at(identifier);
    }

    map<string, vector<double>> get_all_forces() {
        return this->external_forces;
    }

    vector<Node> *get_nodes() {
        return this->nodes;
    }

    list<Edge> *get_edges() {
        return this->edges;
    }

    double get_edge_deform_at() {
        return this->edge_deform_at;
    }

    double get_edge_tear_at() {
        return this->edge_tear_at;
    }
};

#endif