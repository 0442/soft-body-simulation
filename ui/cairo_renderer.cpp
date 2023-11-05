#include <bits/stdc++.h>
#include <cairo/cairo-xlib.h>
#include <X11/Xlib.h>
#include "../utils/vectors.cpp"
#include "renderers.h"

#ifndef UI_CAIRO_RENDERER_CPP_
#define UI_CAIRO_RENDERER_CPP_

using namespace std;
using namespace utils::vectors;

tuple<Display *, Drawable, int> CairoRenderer::init_x11(int w, int h)
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

cairo_surface_t *CairoRenderer::init_cairo(Display *dsp, Drawable da, int screen, int x, int y)
{
    cairo_surface_t *sfc;

    sfc = cairo_xlib_surface_create(dsp, da, DefaultVisual(dsp, screen), x, y);
    cairo_xlib_surface_set_size(sfc, x, y);

    return sfc;
}

tuple<cairo_t *, cairo_surface_t *> CairoRenderer::init_surface(Display *dsp, Drawable drw, int screen)
{
    cairo_surface_t *sfc = init_cairo(dsp, drw, screen, WIDTH, HEIGHT);
    cairo_t *cr = cairo_create(sfc);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_set_font_size(cr, 20);
    cairo_set_line_width(cr, 1);
    cairo_save(cr);

    return make_tuple(cr, sfc);
}

CairoRenderer::CairoRenderer() {}
CairoRenderer::CairoRenderer(double width_m, double height_m) : _BaseRenderer() {
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

void CairoRenderer::begin() { cairo_push_group(this->cr); }
void CairoRenderer::add_line(vector<double> pos1, vector<double> pos2, double width, color_t color) {
    pos1 = scale_vector(pos1, this->m_to_px);
    pos2 = scale_vector(pos2, this->m_to_px);
    width *= this->m_to_px;

    cairo_set_source_rgb(this->cr, color.r / 255.0, color.g / 255.0, color.b / 255.0);
    cairo_set_line_width(this->cr, width);
    cairo_move_to(this->cr, pos1[0], pos1[1]);
    cairo_line_to(this->cr, pos2[0], pos2[1]);
    cairo_stroke(this->cr);
};

void CairoRenderer::add_rectangle(vector<double> pos, double width, double height, color_t color) {
    pos = scale_vector(pos, this->m_to_px);
    width *= this->m_to_px;
    height *= this->m_to_px;

    cairo_set_source_rgb(this->cr, color.r / 255.0, color.g / 255.0, color.b / 255.0);
    cairo_rectangle(this->cr, pos[0], pos[1], width, width);
    cairo_fill(this->cr);
};

void CairoRenderer::add_circle(vector<double> pos, double r, color_t color) {
    pos = scale_vector(pos, this->m_to_px);
    r *= this->m_to_px;

    cairo_set_source_rgb(this->cr, color.r / 255.0, color.g / 255.0, color.b / 255.0);
    cairo_arc(this->cr, pos[0], pos[1], r, 0, 2 * M_PI);
    cairo_fill(this->cr);
}

void CairoRenderer::render() {
    cairo_pop_group_to_source(this->cr);
    cairo_paint(this->cr);
    cairo_surface_flush(this->srfc);
    XFlush(this->dsp);
};

void CairoRenderer::quit() {
    XCloseDisplay(this->dsp);
}

#endif