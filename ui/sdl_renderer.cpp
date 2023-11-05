#include <SDL2/SDL.h>
#include "renderers.h"

SDLRenderer::SDLRenderer() : _BaseRenderer() {
    if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        cout << "error in SDL_InitSubSystem" << endl;
        exit(1);
    }
    int status = SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE, &win, &rndr);
    if (status != 0) {
        cout << "error in SDL_CreateWindowAnd_BaseRenderer" << endl;
        exit(1);
    }

    SDL_Surface *win_surf = SDL_GetWindowSurface(win);
}

void SDLRenderer::add_line(vector<double> pos1, vector<double> pos2, double width, color_t color) {
    SDL_SetRenderDrawColor(rndr, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLineF(rndr, pos1[0], pos1[1], pos2[0], pos2[1]);
}

void SDLRenderer::add_circle(vector<double> pos, double radius){ }

void SDLRenderer::add_rect(vector<double> pos, vector<double> size, color_t color) {
    /*SDL_SetRenderDrawColor(rndr, color.r, color.g, color.b, color.a);
    SDL_Rect r = {pos[0], pos[1], size[0], size[1]};
    SDL_RenderFillRect(rndr, &r);*/
}

void SDLRenderer::render() {
    SDL_RenderPresent(rndr);
    SDL_RenderClear(rndr);
    SDL_UpdateWindowSurface(win);
}