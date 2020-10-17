#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include <cstdint>
#include <SDL2/SDL.h>

class Context {
    SDL_Window*     window    = nullptr;
    SDL_Renderer*   renderer  = nullptr;
    const char*     title     = "Snake Game";
    const uint32_t win_x_pos  = SDL_WINDOWPOS_CENTERED;
    const uint32_t win_y_pos  = SDL_WINDOWPOS_CENTERED;
    const uint32_t win_width  = 800;
    const uint32_t win_height = 640;
public:
    Context(void);
    ~Context(void);

    void copy_and_render(SDL_Texture*, SDL_Rect*);
    void clear_renderer(void);

    [[noreturn]] void quit_on_error(const char*) const;

    SDL_Window*   get_window(void) const   { return window; }
    uint32_t      get_height(void) const   { return win_height; }
    uint32_t      get_width(void) const    { return win_width; }
    SDL_Renderer* get_renderer(void) const { return renderer; }
    void          render_present(void)     { SDL_RenderPresent(renderer); }
};

#endif /* _CONTEXT_H_ */
