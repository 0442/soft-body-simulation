#include <bits/stdc++.h>
#include "./utils/vector_operations.hpp"
#include "softbody.hpp"

#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

using namespace std;

template <typename T>
int sign(T n)
{
    return (0 < n) - (n < 0);
}

class Simulator
{
private:
    double bounce_coef;
    double friction_coef;
    double node_pull_spring_coef = 10;
    vector<SoftBody *> bodies;

public:
    double dsp_w_m = 5;
    double dsp_h_m = 5;

    Simulator(){};
    Simulator(double bounce_coef, double friction_coef)
    {
        this->bounce_coef = bounce_coef;
        this->friction_coef = friction_coef;
    }

    void handle_wall_collisions()
    {
        double disp_w = this->dsp_w_m;
        double disp_h = this->dsp_h_m;
        vector<double> normal_f, friction_f;
        vector<double> a, v, p;

        for (SoftBody *b_ptr : this->bodies)
        {
            vector<Node> *nodes = b_ptr->get_nodes();
            vector<Node>::iterator n_ptr = nodes->begin();
            for (n_ptr; n_ptr != nodes->end(); n_ptr++)
            {
                a = n_ptr->get_acceleration();
                v = n_ptr->get_velocity();
                p = n_ptr->get_position();
                normal_f = {0, 0};
                friction_f = {0, 0};

                // floor/ceiling collision
                if (p[1] >= disp_h || p[1] <= 0)
                {
                    normal_f = {0, -1 * n_ptr->force_sum()[1]};
                    friction_f = {-sign(v[0]) * vector_len(normal_f) * this->friction_coef, 0};

                    a[1] = 0;
                    v[1] = -1 * v[1] * this->bounce_coef;

                    if (p[1] > 0)
                        p[1] = disp_h;
                    else
                        p[1] = 0;
                }
                // right/left wall collision
                if (p[0] >= disp_w || p[0] <= 0)
                {
                    normal_f = {-1 * n_ptr->force_sum()[0], 0};
                    friction_f = {0, -sign(v[1]) * vector_len(normal_f) * this->friction_coef};

                    a[0] = 0;
                    v[0] = -1 * v[0] * this->bounce_coef;

                    if (p[0] > 0)
                        p[0] = disp_w;
                    else
                        p[0] = 0;
                }

                n_ptr->set_force("normal", normal_f);
                n_ptr->set_force("friction", friction_f);
                n_ptr->set_acceleration(a);
                n_ptr->set_velocity(v);
                n_ptr->set_position(p);
            }
        }
    }

    void add_body(SoftBody *body)
    {
        this->bodies.push_back(body);
    }

    void __apply_air_resistance() {}

    void simulate_next_frame(double time_step_s)
    {
        handle_wall_collisions();

        for (SoftBody *b_ptr : this->bodies)
        {
            b_ptr->advance_physics(time_step_s);
        }
    }

    void get_all_nodes(vector<Node*> *nodes)
    {
        for (auto b : this->bodies) {
            vector<Node> *ns = b->get_nodes();
            vector<Node>::iterator n_itr = ns->begin();
            while (n_itr != ns->end()) {
                nodes->push_back(&*n_itr);
                n_itr++;
            }
        }
    }
    void get_all_edges(vector<Edge*> *edges)
    {
        for (auto b : this->bodies) {
            list<Edge> *es = b->get_edges();
            list<Edge>::iterator e_itr = es->begin();
            while (e_itr != es->end()) {
                edges->push_back(&*e_itr);
                e_itr++;
            }
        }
    }
};

#endif