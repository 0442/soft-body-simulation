from math import sqrt
from functools import reduce
import numpy
from utils import vector_len, vector_sum, vector_sub, vector_len, unit_vec, scale_vector
from constants import TIME_STEP
#from decimal import *

# TODO
# - do calculations with fixed point accuracy
# - bug: spinning objects start gaining speed infinitely
# - air resistance

class Node:
    """ Represents a point mass.
    """
    def __init__(self, x:float, y:float, mass:float) -> 'Node':
        self.i = None
        self.__mass = mass

        self.__position = (x,y)
        self.__velocity = (0,0)
        self.__acceleration = (0,0)

        self.__forces = {}

    def set_force(self, identifier, vector:tuple[float,float]):
        self.__forces[identifier] = vector

    def remove_force(self, identifier):
        self.__forces.pop(identifier)

    def set_velocity(self, velocity_vector:tuple[float,float]):
        self.__velocity = velocity_vector

    def set_acceleration(self, acceleration_vector:tuple[float,float]):
        self.__acceleration = acceleration_vector

    def set_position(self, position_vector:tuple[float,float]):
        self.__position = position_vector

    def force_sum(self):
        return reduce(
            vector_sum,
            self.__forces.values(),
        )

    def update_state(self):
        mass = self.__mass
        vel = self.__velocity
        pos = self.__position
        a = self.__acceleration
        force_sum = self.force_sum()

        new_a = scale_vector(force_sum, 1/mass)
        avg_a = scale_vector(vector_sum(a, new_a), 1/2)

        new_vel = vector_sum(vel, scale_vector(avg_a, TIME_STEP))
        avg_vel = scale_vector(vector_sum(vel, new_vel), 1/2)
        new_pos = vector_sum(pos, scale_vector(avg_vel, TIME_STEP))

        self.__velocity = new_vel
        self.__acceleration = new_a
        self.__position = new_pos

    @property
    def position(self):
        return self.__position
    @property
    def velocity(self):
        return self.__velocity
    @property
    def acceleration(self):
        return self.__acceleration
    @property
    def mass(self):
        return self.__mass

class Edge:
    def __init__(self, node1:Node, node2:Node, spring_const:float, damping_const:float, rest_length:float) -> 'Edge':
        self.__node1 = node1
        self.__node2 = node2

        self.__spring_const = spring_const
        self.__damping_const = damping_const
        self.__rest_length = rest_length

        self.__deformation = None
        self.update_deformation()

    def update_deformation(self):
        diff_vector = (
            abs(self.__node2.position[0] - self.__node1.position[0]),
            abs(self.__node2.position[1] - self.__node1.position[1])
        )
        spring_length = vector_len(diff_vector)

        self.__deformation = spring_length - self.__rest_length

    def calculate_spring_force(self):
        """Returns the spring force vectors based on current states of nodes

        Calculated based on current state of edge and nodes
        """
        self.update_deformation()
        magnitude = self.__deformation * self.__spring_const

        distance_vect = (
            self.__node2.position[0] - self.__node1.position[0],
            self.__node2.position[1] - self.__node1.position[1]
        )
        distance = vector_len(distance_vect)
        scale_factor = magnitude/distance if distance != 0 else 0

        force_vect_1 = scale_vector(distance_vect, scale_factor)
        force_vect_2 = scale_vector(force_vect_1, -1)

        return force_vect_1, force_vect_2

    def calculate_damping_vectors(self):
        """Returns velocity vectors for damping the springing between nodes.

        Calculated based on current state of edge and nodes
        """
        vel1 = self.__node1.velocity
        vel2 = self.__node2.velocity

        relative_pos1 = unit_vec(vector_sub(self.__node2.position, self.__node1.position))
        relative_pos2 = unit_vec(vector_sub(self.__node1.position, self.__node2.position))

        relative_vel1 = vector_sub(vel2, vel1)
        relative_vel2 = vector_sub(vel1, vel2)

        d1 = numpy.dot(relative_pos1, relative_vel1)
        d1 = numpy.dot(relative_pos2, relative_vel2)
        damp_vel1 = scale_vector(relative_pos1, d1 * self.__damping_const)
        damp_vel2 = scale_vector(damp_vel1, -1)

        f1, f2 = self.calculate_spring_force()
        angle1 = numpy.arccos(numpy.dot(f1,damp_vel1) / (vector_len(f1) * vector_len(damp_vel1)))
        angle2 = numpy.arccos(numpy.dot(f2,damp_vel2) / (vector_len(f2) * vector_len(damp_vel2)))
        if angle1 > 1:
            damp_vel1 = (0,0)
        if angle2 > 1:
            damp_vel2 = (0,0)


        return damp_vel1, damp_vel2

    def set_rest_length(self, new_rest_length):
        """Mainly for deformation"""
        self.__rest_length = new_rest_length

    @property
    def start_node(self) -> Node:
        return self.__node1
    @property
    def end_node(self) -> Node:
        return self.__node2
    @property
    def spring_const(self) -> float:
        return self.__spring_const
    @property
    def damping_const(self) -> float:
        return self.__damping_const
    @property
    def deformation(self) -> float:
        return self.__deformation
    @property
    def rest_length(self) -> float:
        return self.__rest_length


class SoftBody:
    """Represents a soft body, a net made of nodes (point masses) and edges (spring forces).
    """
    def __init__(self, nodes:list[Node], edge_spring_const:float, edge_damping_const:float, edge_rest_length:float, edge_deform_deform:float=float('inf') ,edge_tear_deform:float=float('inf')):
        #TODO
        # - damping

        self.__nodes:list[Node] = nodes
        self.__edges:list[Edge] = []

        self.__edge_spring_const = edge_spring_const
        self.__edge_damping_const = edge_damping_const
        self.__edge_rest_length = edge_rest_length
        self.__edge_tear_deform = edge_tear_deform
        self.__edge_deform_deform = edge_deform_deform

        self.__external_forces = {}

    def add_edge(self, node1_index, node2_index, spring_const=None, damping_const=None, rest_length=None):
        """Adds an edge (spring) between two nodes.

        Custom values for spring_const, damping_const and rest_length can be given to each edge.
        If left unchanged, they default to values assigned to
            edge_spring_const,
            edge_damping_const
            edge_rest_length
        at object creation.
        """
        if spring_const == None:
            spring_const = self.__edge_spring_const
        if damping_const == None:
            damping_const = self.__edge_damping_const
        if rest_length == None:
            rest_length = self.__edge_rest_length

        node1 = self.__nodes[node1_index]
        node2 = self.__nodes[node2_index]

        if rest_length == "distance":
            p1 = node1.position
            p2 = node2.position
            rest_length = sqrt((p2[0]-p1[0])**2 + (p2[1]-p1[1])**2)

        edge = Edge(node1, node2, spring_const, damping_const, rest_length)
        self.__edges.append(edge)

    def add_external_force(self, name:str, force_vector:tuple):
        """Adds a force that acts on all the nodes equally all the time.

        Raises ValueError if a force with given name already exits.
        """
        if name in self.__external_forces.keys():
            raise ValueError("A force with the given name is already registered.")

        self.__external_forces[name] = force_vector

    def advance_physics(self):
        """Calculate new states for Nodes for the next time step.
        """
        # springs/edges
        for edge in list(self.__edges):
            node1 = edge.start_node
            node2 = edge.end_node

            # tear edge
            if abs(edge.deformation) > self.__edge_tear_deform:
                node1.remove_force(edge)
                node2.remove_force(edge)
                self.__edges.remove(edge)
                continue

            # deform edge
            if abs(edge.deformation) > self.__edge_deform_deform:
                edge.set_rest_length(edge.rest_length + edge.deformation)

            force1, force2 = edge.calculate_spring_force()

            node1.set_force(edge, force1)
            node2.set_force(edge, force2)

            # damping
            d_vel1, d_vel2 = edge.calculate_damping_vectors()
            vel1 = vector_sum(node1.velocity, d_vel1)
            vel2 = vector_sum(node2.velocity, d_vel2)
            node1.set_velocity(vel1)
            node2.set_velocity(vel2)

        # add external forces and update nodes
        for node in self.__nodes:
            for name, force in self.__external_forces.items():
                node.set_force(name, force)

            node.update_state()

    def add_velocity(self, vector:tuple[float]):
        """Add velocity for every node, changing the whole body's movement.
        """
        for node in self.__nodes:
            new_vel = node.velocity
            new_vel = ( new_vel[0] + vector[0], new_vel[1] + vector[1] )
            node.set_velocity(new_vel)

    def move_body(self, top_left_pos:tuple[float]):
        """Move object's top left corner to given position all other nodes along with it.
        """
        object_top_left = reduce(
            lambda tl,node: ( min(tl[0],node.position[0]), min(tl[1],node.position[1]) ),
            self.__nodes,
            (float('inf'), float('inf'))
        )

        transf_vect = (
            top_left_pos[0] - object_top_left[0],
            top_left_pos[1] - object_top_left[1]
        )

        for node in self.__nodes:
            pos = node.position
            node.set_position((
                pos[0] + transf_vect[0],
                pos[1] + transf_vect[1]
            ))

    def get_force(self, name:str):
        """Return the force vector of force with give name.

        Returns None if there is no matching force.
        """
        return self.__external_forces[name]

    def get_all_forces(self):
        """Returns a dict of all forces and their vectors."""
        return self.__external_forces

    @property
    def nodes(self):
        return self.__nodes
    @property
    def edges(self):
        return self.__edges
    @property
    def edge_tear_deformation(self):
        return self.__edge_tear_deform
    @property
    def edge_deform_deformation(self):
        return self.__edge_deform_deform
