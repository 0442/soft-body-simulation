#include <bits/stdc++.h>
#include "softbody/softbody.h"
#include "simulator.h"
#include "ui/ui.cpp"

using namespace std;

vector<double> p_args(int argc, char **argv) {
    if (argc < 7) {
        cout << "Required arguments: \n"
             << "    <spring> <damping> <friction> <time step> <time scale> <frame rate>" << endl;
        exit(1);
    }

    vector<double> args;
    for (int i = 1; i < argc; i++) {
        args.push_back(atof(argv[i]));
    }
    return args;
}

int main(int argc, char **argv)
{
    auto args = p_args(argc, argv);
    double spring_coef = args[0];
    double damping_coef = args[1];
    double friction_coef = args[2];
    double time_step = args[3];
    double time_scale = args[4];
    uint frame_rate = args[5];

    vector<Node> node_v = {
        Node({1, 0.1}, 0.2),
        Node({1.6, 0.1}, 0.2),
        Node({2.2, 0.1}, 0.2),
        Node({2.8, 0.1}, 0.2),
        Node({1, 0.7}, 0.2),
        Node({1.6, 0.7}, 0.2),
        Node({2.2, 0.7}, 0.2),
        Node({2.8, 0.7}, 0.2),
    };
    cout << "hello" << endl;

    list<Edge> edge_l = {
        Edge(&node_v[0], &node_v[1], spring_coef, damping_coef),
        Edge(&node_v[1], &node_v[2], spring_coef, damping_coef),
        Edge(&node_v[2], &node_v[3], spring_coef, damping_coef),
        Edge(&node_v[3], &node_v[7], spring_coef, damping_coef),
        Edge(&node_v[7], &node_v[6], spring_coef, damping_coef),
        Edge(&node_v[6], &node_v[5], spring_coef, damping_coef),
        Edge(&node_v[5], &node_v[4], spring_coef, damping_coef),
        Edge(&node_v[4], &node_v[0], spring_coef, damping_coef),

        Edge(&node_v[0], &node_v[5], spring_coef, damping_coef),
        Edge(&node_v[1], &node_v[4], spring_coef, damping_coef),
        Edge(&node_v[1], &node_v[6], spring_coef, damping_coef),
        Edge(&node_v[2], &node_v[5], spring_coef, damping_coef),
        Edge(&node_v[2], &node_v[7], spring_coef, damping_coef),
        Edge(&node_v[3], &node_v[6], spring_coef, damping_coef),

        Edge(&node_v[1], &node_v[5], spring_coef, damping_coef),
        Edge(&node_v[2], &node_v[6], spring_coef, damping_coef)};

    ios_base::sync_with_stdio(true);

    SoftBody sb = SoftBody(&node_v, &edge_l, 2, 1, 0.5);
    SoftBody sb2 = SoftBody(node_v, edge_l);

    //sb.add_velocity({0, 0});
    //sb.move_relative({1, 4.3});
    sb.move_relative({1, 1.3});
    vector<double> g = {0, 9.81 * node_v[0].get_mass()};
    sb.set_external_force("gravity", g);

    Simulator s = Simulator(0, friction_coef);
    s.add_body(&sb);

    Ui<CairoRenderer> u = Ui<CairoRenderer>(&s, time_scale);
    u.simulation_auto_run(time_step, frame_rate);
    return 0;
}
