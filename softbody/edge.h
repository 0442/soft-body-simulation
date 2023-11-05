#include <bits/stdc++.h>
#include "node.h"

#ifndef SOFTBODY_EDGE_H_
#define SOFTBODY_EDGE_H_

using namespace std;

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
        Edge();

        Edge(Node node1, Node node2, double spring_coef, double damping_coef, double rest_length);
        // if rest_length not given, set rest_length as the current distance of nodes 1 and 2.
        Edge(Node node1, Node node2, double spring_coef, double damping_coef);
        Edge(Node *node1, Node *node2, double spring_coef, double damping_coef);

        //~Edge();
        //Edge(const Edge &e);

        void update_deformation();

        pair<vector<double>, vector<double>> calculate_spring_force();
        pair<vector<double>, vector<double>> calculate_damping_vectors();

        void set_rest_length(double new_rest_length);

        Node *get_node1();
        Node *get_node2();
        double get_spring_coef();
        double get_damping_coef();
        double get_deformation();
        double get_rest_length();

        // id used for distinguishing between other edges, e.g. when there are multiple spring/edge forces acting on one node.
        void set_id(string id);
        string get_edge_id();
};

#endif