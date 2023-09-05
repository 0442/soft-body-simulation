#include "softbody.hpp"
#include <bits/stdc++.h>
#include "simulator.hpp"
#include "./ui/ui.hpp"

using namespace std;

double spring_coeff = 40;
double damping_coeff = 0.07;

vector<Node> node_v = {
    Node({1, 0.1}, 0.05),
    Node({1.6, 0.1}, 0.05),
    Node({2.2, 0.1}, 0.05),
    Node({2.8, 0.1}, 0.05),
    Node({1, 0.7}, 0.05),
    Node({1.6, 0.7}, 0.05),
    Node({2.2, 0.7}, 0.05),
    Node({2.8, 0.7}, 0.05),
};

list<Edge> edge_l = {
    Edge(&node_v[0], &node_v[1], spring_coeff, damping_coeff),
    Edge(&node_v[1], &node_v[2], spring_coeff, damping_coeff),
    Edge(&node_v[2], &node_v[3], spring_coeff, damping_coeff),
    Edge(&node_v[3], &node_v[7], spring_coeff, damping_coeff),
    Edge(&node_v[7], &node_v[6], spring_coeff, damping_coeff),
    Edge(&node_v[6], &node_v[5], spring_coeff, damping_coeff),
    Edge(&node_v[5], &node_v[4], spring_coeff, damping_coeff),
    Edge(&node_v[4], &node_v[0], spring_coeff, damping_coeff),

    Edge(&node_v[0], &node_v[5], spring_coeff, damping_coeff),
    Edge(&node_v[1], &node_v[4], spring_coeff, damping_coeff),
    Edge(&node_v[1], &node_v[6], spring_coeff, damping_coeff),
    Edge(&node_v[2], &node_v[5], spring_coeff, damping_coeff),
    Edge(&node_v[2], &node_v[7], spring_coeff, damping_coeff),
    Edge(&node_v[3], &node_v[6], spring_coeff, damping_coeff),

    Edge(&node_v[1], &node_v[5], spring_coeff, damping_coeff),
    Edge(&node_v[2], &node_v[6], spring_coeff, damping_coeff)};

int main()
{
    SoftBody sb = SoftBody(&node_v, &edge_l);
    sb.add_velocity({0, 0});
    vector<double> g = {0, 9.81 * node_v[0].get_mass()};
    sb.set_external_force("gravity", g);

    Simulator s = Simulator(0.0, 0.01);
    s.add_body(&sb);

    Ui<CairoRenderer> u = Ui<CairoRenderer>(&s);
    u.simulation_auto_run(10);
    return 0;
}