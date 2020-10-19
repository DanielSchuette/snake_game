#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>

#include "utils.hh"
#include "snake.hh"

uint32_t speed = 1000 / 20;
char* player_name;

int main(int argc, char** argv)
{
    if (argc != 3) {
        std::cerr << "usage: snake <player_name> <speed>\n";
        exit(1);
    }
    player_name = *(++argv);

    int div = atoi(*(++argv));
    if (div == 0 || div > 50) {
        std::cerr << "error: invalid speed (0 < speed <= 50)\n";
        exit(1);
    }
    speed = 1000 / div;

    const uint32_t square_size = 20;
    Snake snake(square_size);
    snake.set_player_name(player_name);

    bool quit = false;
    bool pause = false;
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_Q:
                    quit = true;
                    break;
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    if (!pause) snake.update_y(-1);
                    break;
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    if (!pause) snake.update_y(1);
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    if (!pause) snake.update_x(-1);
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    if (!pause) snake.update_x(1);
                    break;
                case SDL_SCANCODE_P:
                    pause = !pause;
                    break;
                default:
                    fprintf(stderr, "unhandled event type %d\n", event.type);
                }
            }
        }
        if (!pause) {
            snake.update_head();
            snake.show();
            SDL_Delay(speed);
        }
    }

    return 0;
}
