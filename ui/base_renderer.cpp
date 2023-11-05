#include <bits/stdc++.h>
#include "renderers.h"

#ifndef UI_DRAWING_CPP_
#define UI_DRAWING_CPP_

using namespace std;

_BaseRenderer::_BaseRenderer(){};
_BaseRenderer::_BaseRenderer(double width_m, double height_m) {
    this->dsp_w_m = width_m;
    this->dsp_h_m = height_m;
}

void _BaseRenderer::begin() {};
void _BaseRenderer::add_line(vector<double> pos1, vector<double> pos2, double width, color_t color) {};
void _BaseRenderer::add_circle(vector<double> pos, double radius, color_t color) {};
void _BaseRenderer::add_rectangle(vector<double> pos1, double width, double height, color_t color) {};
void _BaseRenderer::render() {};
void _BaseRenderer::quit() {};

#endif