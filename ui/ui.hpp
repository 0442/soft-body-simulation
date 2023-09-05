#include <bits/stdc++.h>
#include <thread>
#include <chrono>
#include "drawing.hpp"
#include "../simulator.hpp"
#include <X11/Xlib.h>

template <typename _Renderer>
class Ui
{
private:
    struct
    {
        bool is_paused = false;
        bool quit = false;
        Node *node_pulled;
        bool is_pulling = false;
        bool show_nodes = true;
        bool show_edges = true;
    } state;

    float node_r = 0.04;
    float edge_w = 0.02;
    double time_scale = 1;

    Node pulled_node;
    Node highlighted_node;

    _Renderer renderer;
    Simulator *simulator;

public:
    Ui() {}
    Ui(Simulator *simulator)
    {
        this->renderer = _Renderer(simulator->dsp_w_m, simulator->dsp_h_m);
        this->simulator = simulator;
    }

    void draw_edges()
    {
        if (this->state.show_edges == false)
            return;

        Node *n1, *n2;
        vector<double> pos1, pos2;
        vector<Edge *> es;
        this->simulator->get_all_edges(&es);

        for (auto e : es)
        {
            n1 = e->get_node1();
            n2 = e->get_node2();
            pos1 = n1->get_position();
            pos2 = n2->get_position();
            this->renderer.add_line(pos1, pos2, 0.01, {154, 255, 81, 1});
        }
    }
    void draw_nodes()
    {
        if (this->state.show_nodes == false)
        {
            return;
        }
        vector<Node *> nodes;
        this->simulator->get_all_nodes(&nodes);
        for (auto n : nodes)
            this->renderer.add_circle(n->get_position(), this->node_r, {255, 255, 255, 1});
    }

    void draw_vectors()
    {
        vector<Node *> nodes;
        this->simulator->get_all_nodes(&nodes);
        vector<double> pos;
        vector<double> f;
        vector<double> end_pos;
        for (auto n : nodes)
        {
            pos = n->get_position();
            f = n->force_sum();
            if (vector_len(f) < 0.1)
                continue;
            end_pos = vector_sum(pos, scale_vector(f, 0.5));
            this->renderer.add_line(pos, end_pos, this->edge_w, {0, 100, 255, 1});
        }
    }

    void draw_bg()
    {
        this->renderer.add_rectangle({0, 0}, this->simulator->dsp_w_m, this->simulator->dsp_h_m, {10, 12, 25, 1});
    }

    void redraw_canvas()
    {
        this->renderer.begin();
        draw_bg();
        draw_edges();
        draw_nodes();
        // draw_vectors();
        this->renderer.render();
    }

    void _handle_mouse_press(int x, int y)
    {
        vector<Node *> nodes;
        this->simulator->get_all_nodes(&nodes);

        this->state.is_pulling = false;
        for (auto n : nodes)
        {
            auto pos = n->get_position();
            auto nx = (int)(pos[0] * this->renderer.m_to_px);
            auto ny = (int)(pos[1] * this->renderer.m_to_px);
            int click_r = 10;

            if (nx - click_r < x && x < nx + click_r && ny - click_r < y && y < ny + click_r)
            {
                this->state.node_pulled = n;
                this->state.is_pulling = true;
                break;
            }
        }
    }

    void _handle_key_press(int k)
    {
        if (k == 24)
            this->state.quit = true;
    }

    void handle_events()
    {
        XEvent e;

        while (XPending(this->renderer.dsp))
        {
            XNextEvent(this->renderer.dsp, &e);
            switch (e.type)
            {
            case KeyPress:
                this->_handle_key_press(e.xkey.keycode);
                break;

            case ButtonPress:
                this->_handle_mouse_press(e.xbutton.x, e.xbutton.y);
                break;

            case ButtonRelease:
                this->state.is_pulling = false;
                break;

            case 6:
                this->pull_node(e.xmotion.x, e.xmotion.y);
                break;
            }
        }
    }

    void pull_node(int x, int y)
    {
        if (this->state.is_pulling != true)
            return;
        double nx = x / this->renderer.m_to_px;
        double ny = y / this->renderer.m_to_px;
        this->state.node_pulled->set_position({{nx, ny}});
    }

    void simulation_auto_run(int time_step_ms)
    {
        using namespace chrono;
        using namespace this_thread;

        milliseconds sleep_dur((int)(time_step_ms / this->time_scale));

        while (this->state.quit == false)
        {
            time_point<system_clock> now = system_clock::now();
            this->simulator->simulate_next_frame((double)time_step_ms / 1000);
            handle_events();
            sleep_until(now + sleep_dur);
            this->redraw_canvas();
        }
    }

    void simulation_run_frame_by_fram()
    {
        for (;;)
        {
            cout << "press enter for next frame\n";
            getchar();
            s.simulate_next_frame((double)10 / 1000);
            u.redraw_canvas();
        }
    }
};