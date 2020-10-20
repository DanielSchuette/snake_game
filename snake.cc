#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <thread>
#include <vector>

#include "context.hh"
#include "snake.hh"
#include "utils.hh"

static std::random_device         rand_device;
static std::default_random_engine random_generator(rand_device());

SDL_Texture* texture_from_surface(const char* path, const Context& ctx)
{
    SDL_Surface* surface = IMG_Load(path);
    SDL_Texture* texture = nullptr;
    if (surface) {
        texture = SDL_CreateTextureFromSurface(ctx.get_renderer(), surface);
        SDL_FreeSurface(surface);
    }
    if (texture) return texture;
    else         ctx.quit_on_error(SDL_GetError());
}

// Here, we rely on the fact that tiles are squares, not rectangles.
Fruit::Fruit(uint32_t size, uint32_t xmax, uint32_t ymax)
{
    assert(size != 0 && xmax != 0 && ymax != 0);
    rect.h = rect.w = size;

    // probably fragile (subtract 1 because with exact divisibility, we might
    // end up placing a fruit one tile beyond the playing field)
    uint32_t num_tiles_x = xmax / size - 1;
    uint32_t num_tiles_y = ymax / size - 1;
    std::uniform_int_distribution<int> xdist(0, num_tiles_x);
    std::uniform_int_distribution<int> ydist(0, num_tiles_y);

    rect.x = xdist(random_generator) * size;
    rect.y = ydist(random_generator) * size;
    assert((uint32_t)rect.x <= xmax && rect.x >= 0);
    assert((uint32_t)rect.y <= ymax && rect.y >= 0);
}

std::strong_ordering Fruit::operator<=>(const SDL_Rect& other)
{
    if (other.x == rect.x && other.y == rect.y)
        return std::strong_ordering::equal;
    return std::strong_ordering::less;
}

std::strong_ordering operator<=>(const SDL_Rect& r1, const SDL_Rect& r2)
{
    if (r1.x == r2.x && r1.y == r2.y) return std::strong_ordering::equal;
    return std::strong_ordering::less;
}

Snake::Snake(uint8_t speed) : x_dir(speed), speed(speed)
{
    SDL_Rect rect;
    rect.x = rect.y = 0;
    rect.h = rect.w = speed;
    body.emplace_back(rect);
    hiscore_file.open("highscore", std::ios::in | std::ios::out);
    if (!hiscore_file.is_open())
        std::cerr << "unable to open highscore file\n";
}

void Snake::update_head(void)
{
    if (is_paused) return;

    SDL_Rect new_head = body[0];
    new_head.x += x_dir;
    new_head.y += y_dir;

    if (is_head_colliding()) {
        end_game();
    } else if ((uint32_t)new_head.x >= context.get_width() ||
               (uint32_t)new_head.y >= context.get_height()) {
        end_game();
    }

    body.insert(body.begin(), new_head);
    if (fruit <=> new_head != 0) {
        body.pop_back();
    } else {
        play_audio();
        fruit = Fruit(speed, context.get_width(), context.get_height());
        while (is_fruit_colliding(fruit)) {
            if (fruit.rect.x >= speed && fruit.rect.y >= speed) {
                fruit.rect.x -= speed;
                fruit.rect.y -= speed;
            } else {
                fruit = Fruit(speed, context.get_width(), context.get_height());
            }
        }
    }
}

/* IMPROVE: We should really separate rendering from game logic. The snake
 *          object might handle speed and updating its body, but the context
 *          should be handed everything that needs rendering. Maybe a queue,
 *          that's filled with rectangles by the snake and rendered by the
 *          context - then being the only step in `show()'.
 */
void Snake::show(void)
{
    context.clear_renderer();

    SDL_SetRenderDrawColor(context.get_renderer(), 66, 93, 245, 255);
    SDL_RenderFillRect(context.get_renderer(), &fruit.rect);

    uint8_t step = 255 / body.size();
    step = step > 1 ? step : 1;
    uint8_t green = std::max(255-step, 20);
    for (const SDL_Rect& elem: body) {
        SDL_SetRenderDrawColor(context.get_renderer(), 0, green, 0, 255);
        SDL_RenderFillRect(context.get_renderer(), &elem);
        green = std::max(green-step, 20);
    }

    SDL_SetRenderDrawColor(context.get_renderer(), 0, 0, 0, 255);
    SDL_RenderFillRect(context.get_renderer(), get_head());

    std::string text = "Score: " + std::to_string(body.size());
    context.draw_text(text, { 0, 0, 0, 255 }, 0, 0);

    context.render_present();
}

void Snake::update_x(int8_t scale)
{
    if (x_dir != 0 && body.size() != 1) return;
    else if (is_paused) return;
    x_dir = scale * speed;
    y_dir = 0;
}

void Snake::update_y(int8_t scale)
{
    if (y_dir != 0 && body.size() != 1) return;
    else if (is_paused) return;
    y_dir = scale * speed;
    x_dir = 0;
}

bool Snake::is_head_colliding(void)
{
    SDL_Rect head = body[0];
    for (size_t i = 1; i < body.size(); i++)
        if (body[i] <=> head == 0) return true;

    return false;
}

/* When ending a game, we open two text boxes (showing the player's score and
 * the top 5 highscores) and call `exit(0)'. This function is somewhat messy.
 */
void Snake::end_game(void)
{
    play_audio("assets/lose_sound.wav");
    uint32_t score = body.size();
    std::string msg = "Your score is ";
    msg += std::to_string(score);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "You lost!",
                             msg.c_str(), NULL);

    // display the top 5 scores on the screen
    if (hiscore_file.is_open()) {
        save_score_to_file(score);
        ps_pair other_scores = read_scores_from_file();
        std::sort(other_scores.begin(), other_scores.end(), std::greater<>());
        std::string msg = "The five highest scores are:\n";
        for (size_t i = 0; i < 5; i++) {
            if (i < other_scores.size())
                msg += std::to_string(other_scores[i].first) + " (" +
                       other_scores[i].second + ")\n";
        }
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Your score",
                                 msg.c_str(), NULL);
        hiscore_file.close();
    } else {
        std::cerr << "error: unable to write errors to a highscore file\n";
    }
    exit(0);
}

bool Snake::is_fruit_colliding(const Fruit& fruit)
{
    for (const SDL_Rect& elem: body)
        if (fruit.rect <=> elem == 0) return true;
    return false;
}

/* `play_audio()' implements the easiest way to play a sound via the native SDL
 * sound API. It really doesn't optimize anything (not even loading the .wav
 * files), but it works well enough so that the game is still playable.
 */
void Snake::play_audio(const char* audio_path)
{
    auto fn = [&, audio_path]() {
        SDL_AudioSpec wav_spec;
        uint32_t      wav_length;
        uint8_t*      wav_buffer;

        if (SDL_LoadWAV(audio_path, &wav_spec,
                        &wav_buffer, &wav_length) == NULL) {
            std::cerr << "error: unable to load .wav file\n";
            context.quit_on_error(SDL_GetError());
        }

        SDL_AudioDeviceID device_id = SDL_OpenAudioDevice(NULL, 0, &wav_spec,
                                                          NULL, 0);
        SDL_QueueAudio(device_id, wav_buffer, wav_length);
        SDL_PauseAudioDevice(device_id, 0);

        SDL_Delay(3000);   // just a heuristic, no idea how to determine the
                           // length of the sound
        SDL_CloseAudioDevice(device_id);
        SDL_FreeWAV(wav_buffer);
    };
    std::thread audio_thread(fn);
    audio_thread.detach(); // don't block the game
}

ps_pair Snake::read_scores_from_file(void)
{
    hiscore_file.clear();
    hiscore_file.seekg(0, std::ios::beg);

    ps_pair result_vec;
    while (!hiscore_file.eof()) {
        std::string line;
        std::getline(hiscore_file, line);

        std::string value = "";
        std::string name  = "";
        bool after_colon  = false;
        for (char c: line) {
            if (after_colon)  value += c;
            if (c == ':')     after_colon = true;
            if (!after_colon) name += c;
        }
        if (!line.empty()) {
            auto temp = std::make_pair(std::stoi(value), name);
            result_vec.push_back(temp);
        }
    }
    return result_vec;
}

void Snake::save_score_to_file(uint32_t score)
{
    hiscore_file.clear();
    hiscore_file.seekg(0, std::ios::end);
    hiscore_file << player_name << ": " << score << '\n';
}
