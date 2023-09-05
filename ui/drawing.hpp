#include <SDL2/SDL.h>
#include <bits/stdc++.h>
#include <cairo/cairo-xlib.h>
#include <X11/Xlib.h>
#include "../utils/vector_operations.hpp"

#ifndef DRAWING_HPP
#define DRAWING_HPP

#define WIDTH 900
#define HEIGHT 900

using namespace std;

class _BaseRenderer
{
protected:
    uint dsp_w_px;
    uint dsp_h_px;
    double dsp_w_m;
    double dsp_h_m;

public:
    double m_to_px;
    struct color_t
    {
        Uint8 r, g, b;
        double a;
    };
    _BaseRenderer() {}
    _BaseRenderer(double width_m, double height_m)
    {
        this->dsp_w_m = width_m;
        this->dsp_h_m = height_m;
    }

    virtual void add_line(){};
    virtual void add_circle(){};
    virtual void add_rectangle(){};
    virtual void render(){};
};

class TerminalRenderer : public _BaseRenderer
{
private:
    double m_to_px;
    uint dsp_w_px;
    uint dsp_h_px;
    double dsp_w_m;
    double dsp_h_m;

    struct pixel
    {
        color_t color;
        char content;
        int x, y;
    };

    vector<pixel> content;
    vector<string> output_lines;

    void update_canvas_size()
    {
        char *c_str = getenv("COLUMNS");
        char *l_str = getenv("LINES");
        if (c_str == NULL | l_str == NULL)
        {
            // return;
        }
        this->dsp_w_px = 60; // atoi(c_str);
        this->dsp_h_px = 60; // atoi(l_str);
        this->m_to_px = (double)min(this->dsp_h_px, this->dsp_w_px) / min(this->dsp_h_m, this->dsp_w_m);
    }

    vector<int> floor_pos(vector<double> pos)
    {
        vector<int> flrd_pos;
        for (int i = 0; i < pos.size(); i++)
        {
            flrd_pos.push_back((int)(pos[i] * this->m_to_px));
        }
        return flrd_pos;
    }

public:
    TerminalRenderer() {}
    TerminalRenderer(double width_m, double height_m) : _BaseRenderer()
    {
        this->dsp_w_m = width_m;
        this->dsp_h_m = height_m;
        this->m_to_px = this->dsp_h_px / this->dsp_h_m;
        this->update_canvas_size();
        // ios_base::sync_with_stdio(false);
    }
    void add_line(vector<double> pos1, vector<double> pos2, double width, color_t color)
    {
        // broken
        return;
        vector<int> flrd_pos1 = this->floor_pos(pos1);
        vector<int> flrd_pos2 = this->floor_pos(pos2);

        vector<int> diff = vector_sub<int>(flrd_pos2, flrd_pos1);
        int x_diff = diff[0];
        int y_diff = diff[1];

        signed short int line_grow_x = 0;
        signed short int line_grow_y = 0;
        if (x_diff > y_diff)
        {
            if (y_diff != 0)
            {
                line_grow_x = x_diff / y_diff;
                line_grow_y = y_diff / abs(y_diff);
            }
            else
            {
                line_grow_x = x_diff;
                line_grow_y = 0;
            }
        }
        else
        {
            if (x_diff != 0)
            {
                line_grow_x = x_diff / abs(x_diff);
                line_grow_y = y_diff / x_diff;
            }
            else
            {
                line_grow_x = 0;
                line_grow_y = y_diff;
            }
        }
        vector<int> point_pos = flrd_pos1;

        while (point_pos[0] <= flrd_pos2[0] && point_pos[1] <= flrd_pos2[1])
        {
            for (int x = 0; x != line_grow_x; x += line_grow_x / abs(line_grow_x))
            {
                pixel n_px;
                n_px.content = '*';
                n_px.x = point_pos[0];
                n_px.y = point_pos[1];
                this->content.push_back(n_px);
                point_pos[0] += line_grow_x;
            }

            for (int y = 0; y != line_grow_y; y += line_grow_y / abs(line_grow_y))
            {
                pixel n_px;
                n_px.content = '*';
                n_px.x = point_pos[0];
                n_px.y = point_pos[1];
                this->content.push_back(n_px);
                point_pos[1] += line_grow_y;
            }
        }
    }

    void add_rectangle(vector<double> pos, double width, double height, color_t color) {}

    void add_circle(vector<double> pos, double r, color_t color)
    {
        vector<int> flrd_pos = this->floor_pos(pos);

        // don't add if outside
        if (0 > flrd_pos[0] || flrd_pos[0] > dsp_w_px || 0 > flrd_pos[1] || flrd_pos[1] > this->dsp_h_px)
            return;

        // add the sphere
        pixel new_px;
        new_px.content = 'O';
        new_px.x = flrd_pos[0];
        new_px.y = flrd_pos[1];
        this->content.push_back(new_px);
    }

    void render()
    {
        //  init blank output
        int w = this->dsp_w_px;
        string empty_line;
        for (int i = 0; i < this->dsp_w_px; i++)
        {
            empty_line.append(" ");
        }

        for (int line_i = 0; line_i < this->dsp_h_px; line_i++)
        {
            output_lines.push_back(empty_line);
        }

        // map pixels
        for (pixel px : this->content)
        {
            if (px.y >= this->dsp_h_px || px.x >= this->dsp_w_px)
            {
                continue;
            }
            output_lines[px.y][px.x] = px.content;
        }

        // cursor to top left
        cout << "\x1b[H";

        // output result
        for (string line : output_lines)
            cout << line << "|" << '\n';

        fflush(stdout);
        this->content.erase(this->content.begin(), this->content.end());
        this->output_lines.erase(this->output_lines.begin(), this->output_lines.end());
        this->update_canvas_size();
    }
};

class SDLRenderer : public _BaseRenderer
{
private:
    SDL_Window *win;
    SDL_Renderer *rndr;

public:
    SDLRenderer() : _BaseRenderer()
    {
        if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
        {
            cout << "error in SDL_InitSubSystem" << endl;
            exit(1);
        }
        int status = SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE, &win, &rndr);
        if (status != 0)
        {
            cout << "error in SDL_CreateWindowAnd_BaseRenderer" << endl;
            exit(1);
        }

        SDL_Surface *win_surf = SDL_GetWindowSurface(win);
    }

    void add_line(vector<double> pos1, vector<double> pos2, double width, color_t color)
    {
        SDL_SetRenderDrawColor(rndr, color.r, color.g, color.b, color.a);
        SDL_RenderDrawLineF(rndr, pos1[0], pos1[1], pos2[0], pos2[1]);
    }

    void add_circle(vector<double> pos, double radius)
    {
    }

    void add_rect(vector<double> pos, vector<double> size, color_t color)
    {
        SDL_SetRenderDrawColor(rndr, color.r, color.g, color.b, color.a);
        SDL_Rect r = {pos[0], pos[1], size[0], size[1]};
        SDL_RenderFillRect(rndr, &r);
    }

    void render()
    {
        SDL_RenderPresent(rndr);
        SDL_RenderClear(rndr);
        SDL_UpdateWindowSurface(win);
    }
};

class CairoRenderer : public _BaseRenderer
{
protected:
    tuple<Display *, Drawable, int> init_x11(int w, int h)
    {
        Display *dsp;
        Drawable drw;
        int screen;

        if ((dsp = XOpenDisplay(NULL)) == NULL)
            exit(1);

        screen = DefaultScreen(dsp);
        drw = XCreateSimpleWindow(dsp, DefaultRootWindow(dsp),
                                  0, 0, w, h, 0, 0, 0);
        XSelectInput(dsp, drw, ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask | PointerMotionMask);
        XMapWindow(dsp, drw);

        return make_tuple(dsp, drw, screen);
    }

    cairo_surface_t *init_cairo(Display *dsp, Drawable da, int screen, int x, int y)
    {
        cairo_surface_t *sfc;

        sfc = cairo_xlib_surface_create(dsp, da, DefaultVisual(dsp, screen), x, y);
        cairo_xlib_surface_set_size(sfc, x, y);

        return sfc;
    }

    tuple<cairo_t *, cairo_surface_t *> init_surface(Display *dsp, Drawable drw, int screen)
    {
        cairo_surface_t *sfc = init_cairo(dsp, drw, screen, WIDTH, HEIGHT);
        cairo_t *cr = cairo_create(sfc);
        cairo_set_source_rgb(cr, 1, 1, 1);
        cairo_set_font_size(cr, 20);
        cairo_set_line_width(cr, 1);
        cairo_save(cr);

        return make_tuple(cr, sfc);
    }
    cairo_t *cr;
    cairo_surface_t *srfc;
    int screen;

public:
    Display *dsp;
    double m_to_px;

    CairoRenderer() {}
    CairoRenderer(double width_m, double height_m) : _BaseRenderer()
    {
        this->dsp_w_m = width_m;
        this->dsp_h_m = height_m;
        this->m_to_px = WIDTH / width_m;

        auto t1 = this->init_x11(WIDTH, HEIGHT);
        this->dsp = get<0>(t1);
        this->screen = get<2>(t1);
        auto t2 = this->init_surface(get<0>(t1), get<1>(t1), this->screen);
        this->cr = get<0>(t2);
        this->srfc = get<1>(t2);
    }
    void begin() { cairo_push_group(this->cr); }
    void add_line(vector<double> pos1, vector<double> pos2, double width, color_t color)
    {
        pos1 = scale_vector(pos1, this->m_to_px);
        pos2 = scale_vector(pos2, this->m_to_px);
        width *= this->m_to_px;

        cairo_set_source_rgb(this->cr, color.r / 255.0, color.g / 255.0, color.b / 255.0);
        cairo_set_line_width(this->cr, width);
        cairo_move_to(this->cr, pos1[0], pos1[1]);
        cairo_line_to(this->cr, pos2[0], pos2[1]);
        cairo_stroke(this->cr);
    };

    void add_rectangle(vector<double> pos, double width, double height, color_t color)
    {
        pos = scale_vector(pos, this->m_to_px);
        width *= this->m_to_px;
        height *= this->m_to_px;

        cairo_set_source_rgb(this->cr, color.r / 255.0, color.g / 255.0, color.b / 255.0);
        cairo_rectangle(this->cr, pos[0], pos[1], width, width);
        cairo_fill(this->cr);
    };

    void add_circle(vector<double> pos, double r, color_t color)
    {
        pos = scale_vector(pos, this->m_to_px);
        r *= this->m_to_px;

        cairo_set_source_rgb(this->cr, color.r / 255.0, color.g / 255.0, color.b / 255.0);
        cairo_arc(this->cr, pos[0], pos[1], r, 0, 2 * M_PI);
        cairo_fill(this->cr);
    }

    void render()
    {
        cairo_pop_group_to_source(this->cr);
        cairo_paint(this->cr);
        cairo_surface_flush(this->srfc);
        XFlush(this->dsp);
    };
};

#endif