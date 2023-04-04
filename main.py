#!/bin/env python3
from simulator import Simulation
from softbody import SoftBody, Node

if __name__ == "__main__":
    node_list = [
                        #0
                    Node(1,1,0.01),
                # 1                   #2
            Node(0.3,1.3,0.01), Node(1.7,1.3,0.01),
            # 3                         # 4
        Node(0,2,0.01),             Node(2,2,0.01),
                # 5                     # 6
            Node(0.3,2.7,0.01), Node(1.7,2.7,0.01),
                        # 7
                    Node(1,3,0.01),
    ]

    node_list_square = [
        Node(1,1,0.01), Node(1,2,0.01),
        Node(2,1,0.01), Node(2,2,0.01)
    ]

    for node in node_list: node.set_force("gravity", (0, node.mass*9.81))
    for node in node_list_square: node.set_force("gravity", (0, node.mass*9.81))

    soft_body1 = SoftBody(node_list_square, 10, 0.1, 1, edge_deform_deform=float('inf'), edge_tear_deform=1)
    soft_body1.add_edge(0,1, rest_length="distance")
    soft_body1.add_edge(1,3, rest_length="distance")
    soft_body1.add_edge(3,2, rest_length="distance")
    soft_body1.add_edge(2,0, rest_length="distance")
    soft_body1.add_edge(2,1, rest_length="distance")
    soft_body1.add_edge(0,3, rest_length="distance")

    soft_body2 = SoftBody(node_list, 5, 0.05, 1, edge_deform_deform=float('inf'), edge_tear_deform=float('inf'))
    soft_body2.move_body((3.5,4.5))
    soft_body2.add_velocity((2,0))

    soft_body2.add_edge(0,1, rest_length="distance")
    soft_body2.add_edge(1,3, rest_length="distance")
    soft_body2.add_edge(3,5, rest_length="distance")
    soft_body2.add_edge(5,7, rest_length="distance")

    soft_body2.add_edge(0,2, rest_length="distance")
    soft_body2.add_edge(2,4, rest_length="distance")
    soft_body2.add_edge(4,6, rest_length="distance")
    soft_body2.add_edge(6,7, rest_length="distance")

    soft_body2.add_edge(0,5, rest_length="distance")
    soft_body2.add_edge(0,6, rest_length="distance")
    soft_body2.add_edge(1,7, rest_length="distance")
    soft_body2.add_edge(1,4, rest_length="distance")
    soft_body2.add_edge(2,7, rest_length="distance")
    soft_body2.add_edge(2,3, rest_length="distance")
    soft_body2.add_edge(3,6, rest_length="distance")
    soft_body2.add_edge(4,5, rest_length="distance")

    simulation = Simulation(bounce_damping=0.5, friction_coeff=0.1)
    simulation.add_body(soft_body1)
    #simulation.add_body(soft_body2)
    simulation.auto_run()
