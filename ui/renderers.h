#include <bits/stdc++.h>
#include <GLFW/glfw3.h>
#include <cairo/cairo-xlib.h>
#include <X11/Xlib.h>
#include <SDL2/SDL.h>

#ifndef UI_DRAWING_H_
#define UI_DRAWING_H_

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
        uint8_t r, g, b;
        double a;
    };
    _BaseRenderer();
    _BaseRenderer(double width_m, double height_m);

    virtual void begin();
    virtual void add_line(vector<double> pos1, vector<double> pos2, double width, color_t color);
    virtual void add_circle(vector<double> pos, double radius, color_t color);
    virtual void add_rectangle(vector<double> pos1, double width, double height, color_t color);
    virtual void render();
    virtual void quit();
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

    void update_canvas_size();
    vector<int> floor_pos(vector<double> pos);

public:
    TerminalRenderer();
    TerminalRenderer(double width_m, double height_m);
    void begin();
    void add_line(vector<double> pos1, vector<double> pos2, double width, color_t color);
    void add_rectangle(vector<double> pos, double width, double height, color_t color);
    void add_circle(vector<double> pos, double r, color_t color);
    void render();
};

class SDLRenderer : public _BaseRenderer
{
private:
    SDL_Window *win;
    SDL_Renderer *rndr;

public:
    SDLRenderer();
    void add_line(vector<double> pos1, vector<double> pos2, double width, color_t color);
    void add_circle(vector<double> pos, double radius);
    void add_rect(vector<double> pos, vector<double> size, color_t color);
    void render();
};

class CairoRenderer : public _BaseRenderer
{
protected:
    tuple<Display *, Drawable, int> init_x11(int w, int h);
    cairo_surface_t *init_cairo(Display *dsp, Drawable da, int screen, int x, int y);
    tuple<cairo_t *, cairo_surface_t *> init_surface(Display *dsp, Drawable drw, int screen);
    cairo_t *cr;
    cairo_surface_t *srfc;
    int screen;

public:
    Display *dsp;
    double m_to_px;

    CairoRenderer();
    CairoRenderer(double width_m, double height_m);
    void begin();
    void add_line(vector<double> pos1, vector<double> pos2, double width, color_t color);
    void add_rectangle(vector<double> pos, double width, double height, color_t color);
    void add_circle(vector<double> pos, double r, color_t color);
    void render();
    void quit();
};



class OpenGLRenderer: public _BaseRenderer
{
protected:

public:
    double m_to_px;

    GLFWwindow *window;
    GLFWwindow* create_window();
    GLuint shader_program;

    OpenGLRenderer();
    OpenGLRenderer(double width_m, double height_m);
    void begin();
    void add_line(vector<double> pos1, vector<double> pos2, double width, color_t color);
    void add_rectangle(vector<double> pos, double width, double height, color_t color);
    void add_circle(vector<double> pos, double r, color_t color);
    void render();
    void quit();
};

#endif