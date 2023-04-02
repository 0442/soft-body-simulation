import pygame
import sys
from time import sleep, time
from math import sqrt
from collections import namedtuple
from functools import reduce

from decimal import *

class PhysConsts:
    grav_constant = 6.674 * 10**(-10)
    earth_mass = 5.9722 * 10**24
    earth_radius = 6367.4445 * 10**3
    time_step = 0.005

    decimal_precision = 3


class Utils:
    def vector_magnitude(vector:tuple[int]) -> float:
        s = 0
        for c in vector:
            s += c**2
        s = sqrt(s)
        return s

    def num_sign(num:float|int) -> int:
        if num == 0:
            return 0
        sign = int(num/abs(num))
        return sign

    def vector_sum(vect1:tuple[int|float], vect2:tuple[int|float]):
        vect1, vect2 = list(vect1), list(vect2)
        if len(vect2) > len(vect1):
            vect1, vect2 = vect2, vect1

        for i in range(len(vect2)):
            vect1[i] += vect2[i]

        return tuple(vect1)


class Node:
    """ Represents a point mass.
    """
    def __init__(self, x:int, y:int, mass:int) -> 'Node':
        self.i = None
        self.__mass = mass

        self.__position = (x,y)
        self.__velocity = (0,0)
        self.__acceleration = (0,0)

        self.__forces = {}

    def update_force(self, identifier, vector:tuple[int]):
        self.__forces[identifier] = vector

    def remove_force(self, identifier):
        self.__forces.pop(identifier)

    def set_velocity(self, velocity_vector:tuple[int]):
        self.__velocity = velocity_vector

    def set_acceleration(self, acceleration_vector:tuple[int]):
        self.__acceleration = acceleration_vector

    def set_position(self, position_vector:tuple[int]):
        self.__position = position_vector

    def force_sum(self):
        return reduce(
            lambda a, b: (a[0] + b[0], a[1] + b[1]),
            self.__forces.values(),
            (0, 0)
        )

    def update_state(self):
        mass = self.__mass
        vel = self.__velocity
        pos = self.__position
        force_sum = self.force_sum()

        a = (
            force_sum[0] / mass,
            force_sum[1] / mass
        )

        new_vel = (
            vel[0] + a[0] * PhysConsts.time_step,
            vel[1] + a[1] * PhysConsts.time_step
        )

        avg_vel = (
            (vel[0] + new_vel[0]) / 2,
            (vel[1] + new_vel[1]) / 2
        )

        new_pos = (
            pos[0] + avg_vel[0] * PhysConsts.time_step,
            pos[1] + avg_vel[1] * PhysConsts.time_step
        )

        self.__velocity = new_vel
        self.__acceleration = a
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
    def __init__(self, node1:Node, node2:Node, spring_const:int, damping_const:int, rest_length:int) -> 'Edge':
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
        spring_length = Utils.vector_magnitude(diff_vector)

        self.__deformation = spring_length - self.__rest_length

    def calculate_force_vectors(self):
        """Calculates and returns the spring force vectors based on current states of nodes
        """
        self.update_deformation()
        magnitude = self.__deformation * self.__spring_const

        distance_vect = (
            self.__node2.position[0] - self.__node1.position[0],
            self.__node2.position[1] - self.__node1.position[1]
        )
        distance = sqrt(distance_vect[0]**2 + distance_vect[1]**2)
        scale_factor = magnitude/distance if distance != 0 else 0

        force_vect_1 = (
            distance_vect[0] * scale_factor,
            distance_vect[1] * scale_factor
        )
        force_vect_2 = (
            force_vect_1[0] * -1,
            force_vect_1[1] * -1
        )

        return force_vect_1, force_vect_2

    def calculate_damping_vectors(self):
        """Calculates and returns velocity vectors for damping the springing between nodes.
        """
        vel1 = self.__node1.velocity
        vel2 = self.__node2.velocity

        # movement relative to the other node * damping coeff
        damp_vel1 = (
            (vel2[0] - vel1[0]) * self.__damping_const,
            (vel2[1] - vel1[1]) * self.__damping_const,
        )

        damp_vel2 = (
            (vel1[0] - vel2[0]) * self.__damping_const,
            (vel1[1] - vel2[1]) * self.__damping_const,
        )

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
        """Adds an external force that acts on all the nodes equally.

        Raises ValueError if a force with given name already exits.
        """
        if name in self.__external_forces.keys():
            raise ValueError("A force with the given name is already registered.")

        self.__external_forces[name] = force_vector

    def advance_physics(self, time_step:int):
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

            force1, force2 = edge.calculate_force_vectors()

            node1.update_force(edge, force1)
            node2.update_force(edge, force2)

            # damping
            d_vel1, d_vel2 = edge.calculate_damping_vectors()
            vel1 = Utils.vector_sum(node1.velocity, d_vel1)
            vel2 = Utils.vector_sum(node2.velocity, d_vel2)
            node1.set_velocity(vel1)
            node2.set_velocity(vel2)

        # add external forces and update nodes
        for node in self.__nodes:
            for name, force in self.__external_forces.items():
                node.update_force(name, force)

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



class Simulation:
    def __init__(self, bounce_damping:float=0.8, friction_coeff:float=0.5) -> None:
        self.__bg_color = (33,29,27)
        self.__node_color = (235,235,235)
        self.__node_r = 0.06
        self.__edge_w = 0.03

        self.__bounce_damping = bounce_damping
        self.__friction_coeff = friction_coeff

        self.__is_paused = False
        self.__time_scale = 1

        self.__disp_size_pixels = (700,700)
        self.__meter_in_pixels = 100
        self.__disp_size_meters = (
            self.__disp_size_pixels[0] / self.__meter_in_pixels,
            self.__disp_size_pixels[1] / self.__meter_in_pixels
        )

        self.__bodies:list[SoftBody] = []

        pygame.init()
        self.__display = pygame.display.set_mode(self.__disp_size_pixels)

    def collision_detection(self):
        for body in self.__bodies:
            for node in body.nodes:
                x, y = node.position
                disp_w, disp_h = self.__disp_size_meters
                v = node.velocity
                a = node.acceleration

                # floor
                if y > disp_h:
                    friction = (-Utils.num_sign(v[0]) * node.force_sum()[1]* self.__friction_coeff, 0)
                    if abs(v[0]) < 0.5:
                        friction = (0,0)
                        v = (0,v[1])

                    node.set_velocity((
                        v[0],
                        v[1] * self.__bounce_damping * -1
                    ))
                    node.set_acceleration( (a[0], 0) )
                    node.set_position( (x, disp_h) )
                    node.update_force("friction", friction)

                # ceiling
                elif y < 0:
                    friction = (-Utils.num_sign(v[0]) * node.force_sum()[1] * self.__friction_coeff, 0)
                    if abs(v[0]) < 0.5:
                        friction = (0,0)
                        v = (0,v[1])

                    node.set_velocity((
                        v[0],
                        v[1] * self.__bounce_damping * -1
                    ))
                    node.set_acceleration( (a[0], 0) )
                    node.set_position( (x, 0) )
                    node.update_force("friction", friction)

                # right wall
                elif x > disp_w:
                    friction = (0, -Utils.num_sign(v[1]) * node.force_sum()[0] * self.__friction_coeff)
                    if abs(v[1]) < 0.5:
                        friction = (0,0)
                        v = (v[0],0)

                    node.set_velocity((
                        v[0] * self.__bounce_damping * -1,
                        v[1]
                    ))
                    node.set_acceleration( (0, a[1]) )
                    node.set_position( (disp_w, y) )
                    node.update_force("friction", friction)

                # left wall
                elif x < 0:
                    friction = (0, -Utils.num_sign(v[1]) * node.force_sum()[0] * self.__friction_coeff)
                    if abs(v[1]) < 0.5:
                        friction = (0,0)
                        v = (v[0],0)

                    node.set_velocity((
                        -1 * v[0] * self.__bounce_damping,
                        v[1]
                    ))
                    node.set_acceleration( (0, a[1]) )
                    node.set_position( (0, y) )
                    node.update_force("friction", friction)

                else:
                    node.update_force("friction", (0,0))

    def add_body(self, body:SoftBody):
        """add a body"""
        self.__bodies.append(body)

    def advance_simulation(self) -> None:
        """function for advancing the simulation forward one time step."""

        self.collision_detection()
        for b in self.__bodies:
            b.advance_physics(PhysConsts.time_step)

        self.__redraw_canvas()

    def __zoom_out(self):
        if self.__meter_in_pixels <= 10:
            return

        self.__meter_in_pixels -= 10

        self.__disp_size_meters = (
            self.__disp_size_pixels[0] / self.__meter_in_pixels,
            self.__disp_size_pixels[1] / self.__meter_in_pixels
        )

        self.__redraw_canvas()

    def __zoom_in(self):
        if self.__meter_in_pixels >= 200:
            return

        self.__meter_in_pixels += 10

        self.__disp_size_meters = (
            self.__disp_size_pixels[0] / self.__meter_in_pixels,
            self.__disp_size_pixels[1] / self.__meter_in_pixels
        )

        self.__redraw_canvas()

    def __handle_pygame_events(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()


            k = pygame.key.get_pressed()
            if event.type == pygame.KEYDOWN:
                if k[pygame.K_q] == True:
                    pygame.quit()
                    sys.exit()

                if k[pygame.K_SPACE] == True:
                    self.__is_paused = True if self.__is_paused == False else False
                    if self.__is_paused == True: print("paused. [SPACE]")
                    else: print(">>")

            if k[pygame.K_MINUS] == True:
                if self.__time_scale > 0.01:
                    self.__time_scale = round(self.__time_scale - 0.01, 2)
                    print("time scale:", self.__time_scale)

            if k[pygame.K_PLUS] == True:
                if self.__time_scale <= 9.99:
                    self.__time_scale = round(self.__time_scale + 0.01, 2)
                    print("time scale:", self.__time_scale)

            if k[pygame.K_DOWN] == True:
                self.__zoom_out()

            if k[pygame.K_UP] == True:
                self.__zoom_in()

    def auto_run(self, time_scale:float=1) -> None:
        """Loop for advancing the simulation automatically with time scale"""
        self.__time_scale = round(self.__time_scale, 3)
        is_first_loop = True
        while True:
            frame_start_time = time()

            self.__handle_pygame_events()

            if self.__is_paused == False:
                self.advance_simulation()
                frame_end_time = time()
                time_spent = frame_end_time - frame_start_time
                wait = PhysConsts.time_step / self.__time_scale - time_spent
                if wait < -0.01:
                    print(f"simulation cannot keep up! ({round(abs(wait), 5)} seconds behind)")
                elif wait >= 0:
                    sleep(wait)

            # Pause on the first loop i.e. immediately when the simulation is started.
            if is_first_loop== True:
                print("paused. [SPACE]")
                self.__is_paused = True
                is_first_loop = False


    def __draw_nodes(self, body:SoftBody):
        w, h = self.__disp_size_pixels
        for node in body.nodes:
            x, y = node.position
            x = x * self.__meter_in_pixels
            y = y * self.__meter_in_pixels

            pygame.draw.circle(
                self.__display,
                self.__node_color,
                (x, y),
                int(self.__node_r * self.__meter_in_pixels)
            )

    def __draw_edges(self, body:SoftBody):
        w, h = self.__disp_size_pixels
        for edge in body.edges:
            pos1 = edge.start_node.position
            pos2 = edge.end_node.position
            pos1 = (pos1[0] * self.__meter_in_pixels,
                    pos1[1] * self.__meter_in_pixels)
            pos2 = (pos2[0] * self.__meter_in_pixels,
                    pos2[1] * self.__meter_in_pixels)

            cur_deformation = abs(edge.deformation)
            deform_deformation = body.edge_deform_deformation

            red = min(
                (cur_deformation / deform_deformation) * 255,
                255
            ) if deform_deformation != 0 else 0

            color = (red, 255 - red, 0)

            pygame.draw.line(
                self.__display,
                color,
                pos1,
                pos2,
                int(self.__edge_w * self.__meter_in_pixels)
            )

    def __draw_meter_grid(self):
        grid_color = (80,80,80)

        h_line_count, v_line_count = int(self.__disp_size_meters[0]), int(self.__disp_size_meters[0])
        w, h = self.__disp_size_pixels

        # horizontal lines
        for i in range(1, h_line_count + 1):
            y_pos = i * self.__meter_in_pixels
            pygame.draw.line(
                self.__display,
                grid_color,
                (0, y_pos),
                (w, y_pos),
            )
            pygame.font.init()

        # vertical lines
        for i in range(1, v_line_count + 1):
            x_pos = i * self.__meter_in_pixels
            pygame.draw.line(
                self.__display,
                grid_color,
                (x_pos, 0),
                (x_pos, h),
            )

    def __redraw_canvas(self):
        self.__display.fill(self.__bg_color)
        self.__draw_meter_grid()

        for body in self.__bodies:
            self.__draw_edges(body)
            self.__draw_nodes(body)
        pygame.display.update()
