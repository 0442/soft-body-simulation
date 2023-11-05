#include <bits/stdc++.h>

#ifndef SOFTBODY_NODE_H_
#define SOFTBODY_NODE_H_

using namespace std;

class Node {
    private:
        double mass; // allows negative mass
        vector<double> acceleration;
        vector<double> velocity;
        vector<double> position;
        unordered_map<string, vector<double>> forces;

    public:
        Node();
        Node(vector<double> position, double mass);

        void remove_force(string identifier);
        void set_force(string identifier, vector<double> f_vector);
        void set_acceleration(vector<double> acceleration);
        void set_velocity(vector<double> velocity);
        void set_position(vector<double> position);

        void update_state(double time_step);

        double get_mass();
        vector<double> get_acceleration();
        vector<double> get_velocity();
        vector<double> get_position();
        vector<double> get_force(string identifier);
        vector<double> force_sum();
};

#endif