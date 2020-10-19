#ifndef _SNAKE_H_
#define _SNAKE_H_

#include <fstream>
#include <SDL2/SDL.h>
#include <vector>

#include "context.hh"

// player-name + score pair
typedef std::vector<std::pair<uint32_t, std::string>> ps_pair;

std::strong_ordering operator<=>(const SDL_Rect&, const SDL_Rect&);

struct Fruit {
    SDL_Rect rect;

    Fruit(uint32_t, uint32_t, uint32_t);
    void show(void);
    std::strong_ordering operator<=>(const SDL_Rect&);
};

class Snake {
    bool                  is_paused = false;
    uint32_t              x_dir = 0;
    uint32_t              y_dir = 0;
    uint8_t               speed = 20;
    Context               context;
    std::fstream          hiscore_file;
    const char*           player_name;
    Fruit                 fruit = Fruit(speed, context.get_width(),
                                        context.get_height());
    std::vector<SDL_Rect> body;
public:
    Snake(uint8_t);

    void    show(void);
    void    play_audio(const char* = "./assets/crunch.wav");
    void    update_head(void);
    void    update_x(int8_t scale);
    void    update_y(int8_t scale);
    bool    is_head_colliding(void);
    bool    is_fruit_colliding(const Fruit&);
    void    save_score_to_file(uint32_t);
    ps_pair read_scores_from_file(void);

    [[noreturn]] void end_game(void);

    void      toggle_pause(void)             { is_paused = !is_paused; }
    uint32_t  get_win_height(void)           { return context.get_height(); }
    uint32_t  get_win_width(void)            { return context.get_width(); }
    SDL_Rect* get_head(void)                 { return &body[0]; }
    void      set_player_name(const char* n) { player_name = n; }
};

#endif /* _SNAKE_H_ */
