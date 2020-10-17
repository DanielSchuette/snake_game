#include <cstdlib>
#include <iostream>
#include <SDL2/SDL.h>

#include "context.hh"

Context::Context(void)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        quit_on_error(SDL_GetError());

    // X11 usually pings windows to check if they're hung - which we don't need
    bool success = SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_PING, "0");
    if (!success) std::cerr << "unable to set hint\n";

    window = SDL_CreateWindow(title, win_x_pos, win_y_pos, win_width,
                              win_height, 0);
    if (!window) quit_on_error(SDL_GetError());

    uint32_t render_flags = SDL_RENDERER_ACCELERATED;
    renderer = SDL_CreateRenderer(window, -1, render_flags);
    if (!renderer) quit_on_error(SDL_GetError());
}

Context::~Context(void)
{
    if (window)   SDL_DestroyWindow(window);
    if (renderer) SDL_DestroyRenderer(renderer);
}

void Context::copy_and_render(SDL_Texture* texture, SDL_Rect* dest)
{
    SDL_RenderCopy(renderer, texture, NULL, dest);
    render_present();
}

void Context::quit_on_error(const char* msg) const
{
    std::cerr << "error: " << msg << '\n';
    exit(1);
}

// Clear the background. Currently, the color is hard-coded.
void Context::clear_renderer(void)
{
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderClear(renderer);
}
