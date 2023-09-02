#include <bits/stdc++.h>
#include <thread>
#include <chrono>
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <X11/Xlib.h>
#include "./utils/vector_operations.hpp"
#include "softbody.hpp"
#define WIDTH 500
#define HEIGHT 500

using namespace std;

cairo_surface_t *init_cairo(Display *dsp, Drawable da, int screen, int x, int y) {
    cairo_surface_t *sfc;

    sfc = cairo_xlib_surface_create(dsp, da, DefaultVisual(dsp, screen), x, y);
    cairo_xlib_surface_set_size(sfc, x, y);

    return sfc;
}

tuple<Display*, Drawable, int> init_x11(int x, int y) {
    Display *dsp;
    Drawable drw;
    int screen;

    if ((dsp = XOpenDisplay(NULL)) == NULL)
        exit(1);

    screen = DefaultScreen(dsp);
    drw = XCreateSimpleWindow(dsp, DefaultRootWindow(dsp),
        0, 0, x, y, 0, 0, 0);
    XSelectInput(dsp, drw, ButtonPressMask | KeyPressMask);
    XMapWindow(dsp, drw);

    return make_tuple(dsp, drw, screen);
}

tuple<cairo_t*, cairo_surface_t*> init_surface(Display* dsp, Drawable drw, int screen){
    cairo_surface_t *sfc = init_cairo(dsp, drw, screen, 700, 700);
    cairo_t* cr = cairo_create(sfc);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_set_font_size(cr, 20);
    cairo_set_line_width(cr, 1);
    cairo_save(cr);

    return make_tuple(cr, sfc);
}

signed int sign(double n) {
    signed int i = static_cast<signed int>(n);
    return i / abs(i);
}

class Simulation {
    private:
        struct {
            float node_r = 0.08;
            float edge_w = 0.02;

            double m_to_px = 75;
            uint dsp_w_px = 700;
            uint dsp_h_px = 700;
            double dsp_w_m = dsp_w_px / m_to_px;
            double dsp_h_m = dsp_h_px / m_to_px;
        } graphics;

        double bounce_coef;
        double friction_coef;
        double node_pull_spring_coef = 10;
        double time_scale = 0.8;

        Node pulled_node;
        Node highlighted_node;

        vector<SoftBody*> bodies;

        struct {
            bool is_paused = False;
            bool is_pulling_node = False;
            bool show_nodes = True;
            bool show_edges = True;
        } state;

        cairo_t* cr;
        cairo_surface_t *sfc;
        Display *dsp;
        Drawable drw;
        int screen;

    public:
        void setup_screen() {
            auto t1 = init_x11(WIDTH, HEIGHT);
            this->dsp = get<0>(t1);
            this->drw = get<1>(t1);
            this->screen = get<2>(t1);
            tuple<cairo_t *, cairo_surface_t *> t2 = init_surface(this->dsp,this->drw, this->screen);
            this->cr = get<0>(t2);
            this->sfc = get<1>(t2);
        }

        Simulation(double bounce_coef, double friction_coef) {
            this->bounce_coef = bounce_coef;
            this->friction_coef = friction_coef;
            setup_screen();
        }

        void handle_wall_collisions() {
            double disp_w = this->graphics.dsp_w_m;
            double disp_h = this->graphics.dsp_h_m;
            vector<double> normal_f, friction_f;
            vector<double> a, v, p;

            for (SoftBody* b_ptr : this->bodies) {
                vector<Node>* nodes = b_ptr->get_nodes();

                vector<Node>::iterator n_ptr;
                for (n_ptr = nodes->begin(); n_ptr != nodes->end(); n_ptr++) {
                    a = n_ptr->get_acceleration();
                    v = n_ptr->get_velocity();
                    p = n_ptr->get_position();
                    normal_f = {0,0};
                    friction_f = {0,0};

                    // floor/ceiling collision
                    if (p[1] > disp_h || p[1] < 0)
                    {
                        normal_f = {0, -1 * n_ptr->force_sum()[1]};
                        friction_f = {-sign(v[0]) * abs(normal_f[1]) * this->friction_coef, 0};

                        //a[1] = 0;
                        v[1] = -1 * v[1] * this->bounce_coef;

                        if (p[1] > disp_h)
                            p[1] = disp_h;
                        else
                            p[1] = 0;
                    }
                    // left/right wall collision
                    if (p[0] < 0 || p[0] > disp_w) {
                        normal_f = {-1 * n_ptr->force_sum()[0], 0};
                        friction_f = {0, -sign(v[1]) * abs(normal_f[0]) * this->friction_coef};

                        //a[0] = 0;
                        v[0] = -1 * v[0] * this->bounce_coef;

                        if (p[0] > disp_w)
                            p[0] = disp_w;
                        else
                            p[0] = 0;
                    }

                    n_ptr->set_force("normal", normal_f);
                    n_ptr->set_force("friction", friction_f);
                    n_ptr->set_acceleration(a);
                    n_ptr->set_velocity(v);
                    n_ptr->set_position(p);
                }
            }
        }

        void add_body(SoftBody* body) {
            this->bodies.push_back(body);
        }

        void __apply_air_resistance() {}

        void draw_edges(SoftBody* b) {
            if (this->state.show_edges == false) {
                return;
            }
            Node *n1, *n2;
            vector<double> pos1, pos2;
            list<Edge>::iterator e_ptr;
            list<Edge> *es = b->get_edges();
            for (e_ptr = es->begin(); e_ptr != es->end(); e_ptr++)
            {
                n1 = e_ptr->get_node1();
                n2 = e_ptr->get_node2();
                pos1 = n1->get_position();
                pos2 = n2->get_position();
                pos1 = scale_vector(pos1, this->graphics.m_to_px);
                pos2 = scale_vector(pos2, this->graphics.m_to_px);

                cairo_set_source_rgb(this->cr, 0.5, 1, 0.2);
                cairo_set_line_width(this->cr, 3.0);
                cairo_move_to(this->cr, pos1[0], pos1[1]);
                cairo_line_to(this->cr, pos2[0], pos2[1]);
                cairo_stroke(this->cr);

                // do the drawing ...
            }
        }
        void draw_nodes(SoftBody* b) {
            if (this->state.show_nodes == false) {
                return;
            }
            vector<double> pos;
            vector<Node>* nodes = b->get_nodes();
            for (Node n : *nodes)
            {
                pos = n.get_position();
                pos = scale_vector(pos, this->graphics.m_to_px);

                cairo_set_source_rgb(this->cr, 1, 1, 1);
                cairo_arc(this->cr, pos[0], pos[1], this->graphics.node_r*this->graphics.m_to_px, 0, 2 * M_PI);
                //cout << "x: " << pos[0] << " y:" << pos[1] << endl;
                cairo_fill(this->cr);
                // do the drawing ...
            }
            //cout << endl;
        }

        void draw_vectors(SoftBody* b) {
            vector<Node>* nodes = b->get_nodes();
            vector<double> pos;
            vector<double> f;
            vector<double> end_pos;
            for (Node n : *nodes)
            {
                pos = scale_vector(n.get_position(), this->graphics.m_to_px);
                f = n.force_sum();
                end_pos = vector_sum(pos, scale_vector(f, this->graphics.m_to_px * 0.5));

                cairo_set_source_rgb(this->cr, 0, 0, 1);
                cairo_set_line_width(this->cr, 5);
                cairo_move_to(this->cr, pos[0], pos[1]);
                cairo_line_to(this->cr, end_pos[0], end_pos[1]);
                cairo_stroke(this->cr);
            }
        }

        void draw_bg() {
            cairo_set_source_rgb(this->cr, 0.9, 0.6, 0.4);
            cairo_rectangle(this->cr, 0, 0, this->graphics.dsp_w_px, this->graphics.dsp_h_px);
            cairo_fill(this->cr);
        }

        void redraw_canvas() {
            draw_bg();

            for (SoftBody* b : this->bodies) {
                draw_edges(b);
                draw_nodes(b);
                draw_vectors(b);
            }
            XFlush(this->dsp);
        }

        void simulate_next_frame(double time_step) {
            handle_wall_collisions();

            for (SoftBody* b_ptr : this->bodies) {
                b_ptr->advance_physics(time_step);
            }

            redraw_canvas();
        }

        void simulation_auto_run(int time_step) {
            using namespace chrono;
            using namespace this_thread;

            milliseconds sleep_dur((int)(time_step / this->time_scale));

            while (this->state.is_paused == false) {
                time_point<chrono::system_clock> now = system_clock::now();
                simulate_next_frame((double)time_step/1000);
                sleep_until(now += sleep_dur);
                fflush(stdout);
            }
        }
        void test_drawing() {
            chrono::seconds dur(1);
            while (true) {
                cout << "bg" << endl;
                cairo_set_source_rgb(this->cr, 0.8, 0.4, 0.2);
                cairo_rectangle(this->cr, 0, 0, 700, 700);
                cairo_fill(this->cr);
                this_thread::sleep_for(dur);

                cout << "arc" << endl;
                cairo_set_source_rgb(this->cr, 1, 1, 1);
                cairo_set_line_width(this->cr, 10);
                cairo_arc(this->cr, 50.0, 50.0, 20, 0, 2*M_PI);
                cairo_stroke(this->cr);
                this_thread::sleep_for(dur);
                XFlush(this->dsp);
            }
        }
    };