import pygame
import sys
from time import sleep, time
from math import sqrt
from collections import namedtuple
from functools import reduce
import numpy

from decimal import *

# TODO
# - do calculations with fixed point accuracy
# - bug: spinning objects start gaining speed infinitely
# - air resistance

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
        v1, v2 = list(vect1), list(vect2)
        if len(v2) > len(v1):
            v1, v2 = v2, v1

        for i in range(len(v2)):
            v1[i] += v2[i]

        return tuple(v1)

    def vector_sub(vect1:tuple[int|float], vect2:tuple[int|float]):
        v1, v2 = list(vect1), list(vect2)
        if len(v2) > len(v1):
            v1, v2 = v2, v1

        for i in range(len(v2)):
            v1[i] -= v2[i]

        return tuple(v1)

    def vector_len(vector:tuple[float]):
        v = list(vector)
        s = reduce(lambda s,i: s+i**2, v, 0) ** (1/2)
        return s

    def unit_vec(vector:tuple[float]):
        m = 1 / Utils.vector_len(vector)
        n = []
        for i in vector:
            n.append(i * m)
        return n

    def scale_vector(vector:tuple[float], scalar:float):
        v = list(vector)
        n = []
        for i in v:
            n.append(i * scalar)
        return n


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

    def update_force(self, identifier, vector:tuple[float,float]):
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
        spring_length = Utils.vector_magnitude(diff_vector)

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
        """Returns velocity vectors for damping the springing between nodes.

        Calculated based on current state of edge and nodes
        """
        vel1 = self.__node1.velocity
        vel2 = self.__node2.velocity

        relative_pos1 = Utils.unit_vec(Utils.vector_sub(self.__node2.position, self.__node1.position))
        relative_pos2 = Utils.unit_vec(Utils.vector_sub(self.__node1.position, self.__node2.position))

        relative_vel1 = Utils.vector_sub(vel2, vel1)
        relative_vel2 = Utils.vector_sub(vel1, vel2)

        d1 = numpy.dot(relative_pos1, relative_vel1)
        d1 = numpy.dot(relative_pos2, relative_vel2)
        damp_vel1 = Utils.scale_vector(relative_pos1, d1 * self.__damping_const)
        damp_vel2 = Utils.scale_vector(damp_vel1, -1)

        f1, f2 = self.calculate_spring_force()
        angle1 = numpy.arccos(numpy.dot(f1,damp_vel1) / (Utils.vector_len(f1) * Utils.vector_len(damp_vel1)))
        angle2 = numpy.arccos(numpy.dot(f2,damp_vel2) / (Utils.vector_len(f2) * Utils.vector_len(damp_vel2)))
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
        self.__node_r = 0.07
        self.__edge_w = 0.03

        self.__bounce_damping = bounce_damping
        self.__friction_coeff = friction_coeff

        self.__is_paused = False
        self.__is_dragging = False
        self.__dragged_node = None
        self.__highlighted_node = None
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
            b.advance_physics()

        self.__redraw_canvas()

    def __node_clicked(self):
        click_pos = pygame.mouse.get_pos()
        r = self.__node_r * self.__meter_in_pixels

        for b in self.__bodies:
            for n in b.nodes:
                x,y = n.position
                x *= self.__meter_in_pixels
                y *= self.__meter_in_pixels

                if (click_pos[0] - r * 2 <= x <= click_pos[0] + r * 2
                and click_pos[1] - r * 2 <= y <= click_pos[1] + r * 2):
                    self.__dragged_node = n
                    self.__is_dragging = True
                    print("node clicked")
                    return

        self.__is_dragging = False
        self.__dragged_node = None

    def __drag_node(self):
        x, y = pygame.mouse.get_pos()
        x /= self.__meter_in_pixels
        y /= self.__meter_in_pixels
        self.__dragged_node.set_velocity((0,0))
        self.__dragged_node.set_position((x,y))


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

            if event.type == pygame.MOUSEBUTTONDOWN:
                m = pygame.mouse.get_pressed()
                if m[0]:
                    self.__is_dragging = True
            elif event.type == pygame.MOUSEBUTTONUP:
                m = pygame.mouse.get_pressed()
                if not m[0]:
                    self.__is_dragging = False
                    self.__dragged_node = None

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

    def __update_highlighted_node(self):
        mp = pygame.mouse.get_pos()
        r = self.__node_r * self.__meter_in_pixels
        for b in self.__bodies:
            for n in b.nodes:
                x,y = n.position
                x *= self.__meter_in_pixels
                y *= self.__meter_in_pixels
                if (mp[0] - r * 2 < x < mp[0] + r * 2
                and mp[1] - r * 2 < y < mp[1] + r * 2):
                    self.__highlighted_node = n
                    return

        self.__highlighted_node = None

    def auto_run(self) -> None:
        """Loop for advancing the simulation automatically with time scale"""
        self.__time_scale = round(self.__time_scale, 3)
        is_first_loop = True

        frame_start_time = time()

        while True:
            frame_end_time = time()
            time_spent = frame_end_time - frame_start_time
            frame_interval = PhysConsts.time_step / self.__time_scale

            self.__handle_pygame_events()
            self.__update_highlighted_node()

            if self.__is_paused == True and time_spent >= frame_interval:
                frame_start_time = time()
                time_target_deviation = frame_interval - time_spent
                if time_target_deviation < -0.001:
                    print(f"frame {round(abs(time_target_deviation), 5)} seconds behind")

                self.advance_simulation()

            if self.__is_dragging == True:
                if self.__dragged_node == None:
                    self.__node_clicked()

                if self.__dragged_node != None:
                    self.__drag_node()

            # Pause on the first loop i.e. immediately when the simulation is started.
            if is_first_loop== True:
                print("paused. [SPACE]")
                self.__is_paused = True
                is_first_loop = False


    def __draw_nodes(self, body:SoftBody):
        for node in body.nodes:
            x, y = node.position
            x = x * self.__meter_in_pixels
            y = y * self.__meter_in_pixels

            if self.__highlighted_node == node:
                pygame.draw.circle(
                    self.__display,
                    (100,200,200),
                    (x, y),
                    int(self.__node_r * self.__meter_in_pixels * 2)
                )

            pygame.draw.circle(
                self.__display,
                self.__node_color,
                (x, y),
                int(self.__node_r * self.__meter_in_pixels)
            )

    def __draw_edges(self, body:SoftBody):
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

            color = (max(0,red), max(0,255-red), 0)

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