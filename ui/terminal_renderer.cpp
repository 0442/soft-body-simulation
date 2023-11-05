#include <bits/stdc++.h>
#include "renderers.h"
#include "../utils/vectors.cpp"

#ifndef UI_TERMINAL_RENDERER_CPP_
#define UI_TERMINAL_RENDERER_CPP_

using namespace std;
using namespace utils::vectors;

void TerminalRenderer::update_canvas_size()
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

vector<int> TerminalRenderer::floor_pos(vector<double> pos)
{
    vector<int> flrd_pos;
    for (int i = 0; i < pos.size(); i++)
    {
        flrd_pos.push_back((int)(pos[i] * this->m_to_px));
    }
    return flrd_pos;
}

TerminalRenderer::TerminalRenderer() {}
TerminalRenderer::TerminalRenderer(double width_m, double height_m) : _BaseRenderer() {
    this->dsp_w_m = width_m;
    this->dsp_h_m = height_m;
    this->m_to_px = this->dsp_h_px / this->dsp_h_m;
    this->update_canvas_size();
    // ios_base::sync_with_stdio(false);
}
void TerminalRenderer::begin(){}
void TerminalRenderer::add_line(vector<double> pos1, vector<double> pos2, double width, color_t color)
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

void TerminalRenderer::add_rectangle(vector<double> pos, double width, double height, color_t color) {}

void TerminalRenderer::add_circle(vector<double> pos, double r, color_t color)
{
    vector<int> flrd_pos = this->floor_pos(pos);

    // don't add if outside
    if (0 > flrd_pos[0] || flrd_pos[0] > dsp_w_px+1 || 0 > flrd_pos[1] || flrd_pos[1] > this->dsp_h_px+1)
        return;

    // add the sphere
    pixel new_px;
    new_px.content = 'O';
    new_px.x = flrd_pos[0];
    new_px.y = flrd_pos[1];
    this->content.push_back(new_px);
}

void TerminalRenderer::render()
{
    //  init blank output
    int w = this->dsp_w_px;
    string empty_line;
    for (int i = 0; i < this->dsp_w_px; i++)
        empty_line.append(" ");

    for (int line_i = 0; line_i < this->dsp_h_px; line_i++)
        output_lines.push_back(empty_line);

    // map pixels
    for (pixel px : this->content) {
        if (px.y >= this->dsp_h_px || px.x >= this->dsp_w_px)
            continue;
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

#endif