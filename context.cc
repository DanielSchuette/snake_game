#include <cstdlib>
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "context.hh"

Context::Context(void)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) quit_on_error(SDL_GetError());
    if (TTF_Init() != 0)                    quit_on_error(TTF_GetError());

    int img_init_flags = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF;
    if (IMG_Init(img_init_flags) != img_init_flags)
        quit_on_error(IMG_GetError());

    // X11 usually pings windows to check if they're hung - which we don't need
    bool success = SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_PING, "0");
    if (!success) std::cerr << "unable to set hint\n";

    window = SDL_CreateWindow(title, win_x_pos, win_y_pos, win_width,
                              win_height, SDL_WINDOW_SHOWN);
    if (!window) quit_on_error(SDL_GetError());

    uint32_t render_flags = SDL_RENDERER_ACCELERATED;
    renderer = SDL_CreateRenderer(window, -1, render_flags);
    if (!renderer) quit_on_error(SDL_GetError());

    font = TTF_OpenFont("./assets/fonts/OpenSans-Bold.ttf", 24);
    if (!font) quit_on_error(TTF_GetError());
}

Context::~Context(void)
{
    if (window)   SDL_DestroyWindow(window);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (font)     TTF_CloseFont(font);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void Context::quit_on_error(const char* msg) const
{
    std::cerr << "error: " << msg << '\n';
    exit(1);
}

/* Clear the background. Currently, the default color is hard-coded in the
 * header (as a default argument, not a class attribute).
 */
void Context::clear_renderer(SDL_Color c)
{
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_RenderClear(renderer);
}

/* Copy text to the renderer. The caller must still invoke `render_present()'.
 * The standard font set for this context is used.
 */
void Context::draw_text(const std::string& text, const SDL_Color& color,
                        uint32_t x, uint32_t y)
{
    SDL_Surface* sf = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* tx = SDL_CreateTextureFromSurface(renderer, sf);

    SDL_Rect r;
    r.x = x;
    r.y = y;
    SDL_QueryTexture(tx, NULL, NULL, &r.w, &r.h);
    copy_texture_to_renderer(tx, &r);
    SDL_FreeSurface(sf);
    SDL_DestroyTexture(tx);
}

/* Copy a texture to the renderer. The caller must still invoke
 * `render_present()'. We use this function for better naming and convenience,
 * since we usually don't specify any copy flags.
 */
void Context::copy_texture_to_renderer(SDL_Texture* texture, SDL_Rect* dest)
{
    SDL_RenderCopy(renderer, texture, NULL, dest);
}

