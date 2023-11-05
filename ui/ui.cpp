#include <bits/stdc++.h>
#include <thread>
#include <chrono>
#include <X11/Xlib.h>
#include "renderers.h"
#include "../simulator.h"
#include "../utils/vectors.cpp"

#ifndef UI_UI_CPP_
#define UI_UI_CPP_

using namespace utils::vectors;
using namespace std;

template <class _Renderer>
class Ui
{
private:
    struct
    {
        bool is_paused = false;
        bool quit = false;
        Node *node_pulled = NULL;
        bool is_pulling = false;
        bool show_nodes = true;
        bool show_edges = true;
    } state;

    vector<uint> mouse_pos = {0, 0};

    float node_r = 0.04;
    float edge_w = 0.02;
    double time_scale = 1;

    Node pulled_node;
    Node highlighted_node;

    _Renderer renderer;
    Simulator *simulator;
    Simulator *running_simulator;

public:
    Ui() {}
    Ui(Simulator *simulator)
    {
        this->renderer = _Renderer(simulator->dsp_w_m, simulator->dsp_h_m);
        this->simulator = simulator;
    }
    Ui(Simulator *simulator, double time_scale)
    {
        this->renderer = _Renderer(simulator->dsp_w_m, simulator->dsp_h_m);
        this->simulator = simulator;
        this->time_scale = time_scale;
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
            this->renderer.add_line(pos1, pos2, 0.016, {93, 196, 255, 1});
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
            this->renderer.add_circle(n->get_position(), this->node_r, {245, 253, 255, 1});
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
        this->renderer.add_rectangle({0, 0}, this->simulator->dsp_w_m, this->simulator->dsp_h_m, {1, 16, 89, 1});
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
            int click_r = 30;

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
        cout << "key: " << k << endl;
        switch (k)
        {
        case 24:
            this->state.quit = true;
            break;
        case 27:
            this->running_simulator = &*this->simulator;
            break;
        default:
            break;
        }
    }
    void handle_events()
    {
        XEvent e;
        this->pull_node(this->mouse_pos[0], this->mouse_pos[1]);

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
                this->mouse_pos = {e.xmotion.x, e.xmotion.y};
                break;
            }
        }
    }

    void pull_node(int x, int y)
    {
        if (this->state.is_pulling != true) {
            if (this->state.node_pulled != NULL)
                this->state.node_pulled->set_force("pull", {0,0});
            return;
        }
        double nx = x / this->renderer.m_to_px;
        double ny = y / this->renderer.m_to_px;
        auto node_p = this->state.node_pulled->get_position();
        vector<double> pull_f = scale_vector<double>({nx - node_p[0], ny - node_p[1]}, 100*this->state.node_pulled->get_mass());
        this->state.node_pulled->set_force("pull", pull_f);
    }

    void simulation_auto_run(int time_step_ms, uint8_t target_frame_rate)
    {
        using namespace chrono;
        using namespace this_thread;

        int sleep_dur_ms = (time_step_ms / this->time_scale);
        milliseconds sleep_dur(sleep_dur_ms);
        this->running_simulator = &*this->simulator;

        uint frame_counter = 0;
        while (this->state.quit == false)
        {
            //time_point<system_clock> now = system_clock::now();
            this->running_simulator->simulate_next_frame((double)time_step_ms / 1000);
            //sleep_until(now + sleep_dur);
            sleep_for(sleep_dur);

            frame_counter++;
            if (frame_counter*sleep_dur_ms >= (1./target_frame_rate)*1000.)
            {
                this->handle_events();
                this->redraw_canvas();
                frame_counter = 0;
            }
        }

        this->renderer.quit();
    }

    void simulation_run_frame_by_frame(int time_step_ms)
    {
        for (;;)
        {
            cout << "press enter for next frame\n";
            getchar();
            this->simulator->simulate_next_frame((double)time_step_ms / 1000);
            this->redraw_canvas();
        }
    }
};

#endif
