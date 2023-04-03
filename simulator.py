import pygame
import sys
from time import sleep, time
from softbody import SoftBody
from utils import num_sign
from constants import TIME_STEP

class Simulation:
    def __init__(self, bounce_damping:float=1, friction_coeff:float=0.5) -> None:
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
                    friction = (-num_sign(v[0]) * node.force_sum()[1]* self.__friction_coeff, 0)
                    node.set_velocity((
                        v[0],
                        v[1] * self.__bounce_damping * -1
                    ))
                    node.set_acceleration( (a[0], 0) )
                    node.set_position( (x, disp_h) )
                    node.update_force("friction", friction)

                # ceiling
                elif y < 0:
                    friction = (-num_sign(v[0]) * node.force_sum()[1] * self.__friction_coeff, 0)
                    node.set_velocity((
                        v[0],
                        v[1] * self.__bounce_damping * -1
                    ))
                    node.set_acceleration( (a[0], 0) )
                    node.set_position( (x, 0) )
                    node.update_force("friction", friction)

                # right wall
                elif x > disp_w:
                    friction = (0, -num_sign(v[1]) * node.force_sum()[0] * self.__friction_coeff)
                    node.set_velocity((
                        v[0] * self.__bounce_damping * -1,
                        v[1]
                    ))
                    node.set_acceleration( (0, a[1]) )
                    node.set_position( (disp_w, y) )
                    node.update_force("friction", friction)

                # left wall
                elif x < 0:
                    friction = (0, -num_sign(v[1]) * node.force_sum()[0] * self.__friction_coeff)
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
            frame_interval = TIME_STEP / self.__time_scale

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