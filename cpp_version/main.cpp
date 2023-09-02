#include "softbody.hpp"
#include <bits/stdc++.h>
#include "simulation.hpp"

using namespace std;

int main() {
    ios_base::sync_with_stdio(true);
    cout.tie();

    vector<Node>
        node_v = {
            Node({1, 1}, 0.05), Node({1, 2}, 0.05),
            Node({2, 1}, 0.05), Node({2, 2}, 0.05)};

    list<Edge> edge_l = {
        Edge(&node_v[0], &node_v[1], 5, 0.1, 1),
        Edge(&node_v[1], &node_v[3], 5, 0.1, 1),
        Edge(&node_v[3], &node_v[2], 5, 0.1, 1),
        Edge(&node_v[2], &node_v[0], 5, 0.1, 1),
        Edge(&node_v[2], &node_v[1], 5, 0.1, 1),
        Edge(&node_v[0], &node_v[3], 5, 0.1, 1)};

    SoftBody sb = SoftBody(&node_v, &edge_l);
    sb.add_velocity({10, 0});
    vector<double> g = {0, 9.81 * node_v[0].get_mass()};
    sb.set_external_force("gravity", g);

    Simulation s = Simulation(0.0,0.5);
    s.add_body(&sb);

    s.simulation_auto_run(10);
    return 0;

    while (true) {
        getchar();
        cout << "next\n";
        s.simulate_next_frame((double)10/1000);
        s.redraw_canvas();
    }
    //s.test_drawing();
}