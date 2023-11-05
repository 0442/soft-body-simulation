#include <bits/stdc++.h>
#include "./utils/vectors.cpp"
#include "softbody/softbody.h"
#include "simulator.h"

#ifndef SIMULATOR_CPP_
#define SIMULATOR_CPP_

using namespace std;
using namespace utils::vectors;

Simulator::Simulator(){};
Simulator::Simulator(double bounce_coef, double friction_coef)
{
    this->bounce_coef = bounce_coef;
    this->friction_coef = friction_coef;
}

void Simulator::handle_wall_collisions()
{
    double disp_w = this->dsp_w_m;
    double disp_h = this->dsp_h_m;
    vector<double> normal_f, friction_f;
    vector<double> a, v, p;

    for (SoftBody *b_ptr : this->bodies)
    {
        vector<Node> *nodes = b_ptr->get_nodes();
        vector<Node>::iterator n_ptr = nodes->begin();
        for (; n_ptr != nodes->end(); n_ptr++)
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

void Simulator::add_body(SoftBody *body)
{
    this->bodies.push_back(body);
}

void Simulator::__apply_air_resistance() {}

void Simulator::simulate_next_frame(double time_step_s)
{
    for (SoftBody *b_ptr : this->bodies)
        b_ptr->advance_physics(time_step_s);
    handle_wall_collisions();
}

void Simulator::get_all_nodes(vector<Node*> *out)
{
    for (auto b : this->bodies) {
        vector<Node> *ns = b->get_nodes();
        vector<Node>::iterator n_itr = ns->begin();
        while (n_itr != ns->end()) {
            out->push_back(&*n_itr);
            n_itr++;
        }
    }
}
void Simulator::get_all_edges(vector<Edge*> *out)
{
    for (auto b : this->bodies) {
        list<Edge> *es = b->get_edges();
        list<Edge>::iterator e_itr = es->begin();
        while (e_itr != es->end()) {
            out->push_back(&*e_itr);
            e_itr++;
        }
    }
}

#endif