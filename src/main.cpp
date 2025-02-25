#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "raylib.h"
#include "list.h"
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

static double RandomUniform() {
    return (double)rand() / RAND_MAX;
}

const int tile_kind_count = 6;

const auto playfield_size = 10;

static bool in_playfield(int x, int y) {
    return x >= 0 && y >= 0 && x < playfield_size && y < playfield_size;
}

struct Particle {
    double creation_time;
    double lifetime;

    Color color;

    float x;
    float y;

    float velocity_x;
    float velocity_y;
};

struct GameState {
    List<Particle> particles {};

    int tiles[playfield_size][playfield_size];

    bool dragging = false;
    int drag_start_mouse_x;
    int drag_start_mouse_y;
    int drag_start_tile_x;
    int drag_start_tile_y;

    struct FallingTile {
        int x;

        int start_y;
        int end_y;

        int kind;
    };

    bool falling = false;
    List<FallingTile> falling_tiles {};
    float falling_velocity;
    float falling_amount;

    int points = 0;
    int displayed_points = 0;
    double last_displayed_points_tick;
};

static int count_neighbours(GameState state, bool counted[playfield_size][playfield_size], int x, int y, int kind) {
    counted[y][x] = true;

    auto total = 1;

    if(in_playfield(x + 1, y) && state.tiles[y][x + 1] == kind && !counted[y][x + 1]) {
        total += count_neighbours(state, counted, x + 1, y, kind);
    }

    if(in_playfield(x, y + 1) && state.tiles[y + 1][x] == kind && !counted[y + 1][x]) {
        total += count_neighbours(state, counted, x, y + 1, kind);
    }

    if(in_playfield(x - 1, y) && state.tiles[y][x - 1] == kind && !counted[y][x - 1]) {
        total += count_neighbours(state, counted, x - 1, y, kind);
    }

    if(in_playfield(x, y - 1) && state.tiles[y - 1][x] == kind && !counted[y - 1][x]) {
        total += count_neighbours(state, counted, x, y - 1, kind);
    }

    return total;
}

static Color tile_color(int kind) {
    switch(kind) {
        case 1: return RED; break;
        case 2: return GREEN; break;
        case 3: return BLUE; break;
        case 4: return GOLD; break;
        case 5: return SKYBLUE; break;
        case 6: return PURPLE; break;
        default: abort();
    }
}

static void delete_neighbours(GameState *state, double time, int x, int y, int kind) {
    state->tiles[y][x] = 0;

    for(auto i = 0; i < 3; i += 1) {
        auto angle = (float)RandomUniform() * PI * 2;

        append(&state->particles, {
            time,
            0.3 + RandomUniform() * 0.2,
            tile_color(kind),
            x + 0.5f + (float)RandomUniform() * 0.6f - 0.3f,
            y + 0.5f + (float)RandomUniform() * 0.6f - 0.3f,
            cosf(angle) * 5,
            sinf(angle) * 5
        });
    }

    if(in_playfield(x + 1, y) && state->tiles[y][x + 1] == kind) {
        delete_neighbours(state, time, x + 1, y, kind);
    }

    if(in_playfield(x, y + 1) && state->tiles[y + 1][x] == kind) {
        delete_neighbours(state, time, x, y + 1, kind);
    }

    if(in_playfield(x - 1, y) && state->tiles[y][x - 1] == kind) {
        delete_neighbours(state, time, x - 1, y, kind);
    }

    if(in_playfield(x, y - 1) && state->tiles[y - 1][x] == kind) {
        delete_neighbours(state, time, x, y - 1, kind);
    }
}

static int min(int a, int b) {
    if(a > b) {
        return b;
    } else {
        return a;
    }
}

static int max(int a, int b) {
    if(a < b) {
        return b;
    } else {
        return a;
    }
}

const auto window_width = 720;
const auto window_height = 720;

const auto tile_size = 32;

static void playfield_position(int *x, int *y) {
    *x = window_width / 2 - playfield_size * tile_size / 2;
    *y = window_height / 2 - playfield_size * tile_size / 2;
}

static void screen_to_tile(int x, int y, int *tile_x, int *tile_y) {
    int playfield_x;
    int playfield_y;
    playfield_position(&playfield_x, &playfield_y);

    auto relative_x = x - playfield_x;
    auto relative_y = y - playfield_y;

    *tile_x = (int)floorf((float)relative_x / tile_size);
    *tile_y = (int)floorf((float)relative_y / tile_size);
}

static void tile_to_screen(int tile_x, int tile_y, int *x, int *y) {
    auto relative_x = tile_x * tile_size;
    auto relative_y = tile_y * tile_size;

    int playfield_x;
    int playfield_y;
    playfield_position(&playfield_x, &playfield_y);

    *x = playfield_x + relative_x;
    *y = playfield_y + relative_y;
}

const auto tile_inset = 2;

static void draw_tile_at(int x, int y, int kind) {
    auto color = tile_color(kind);

    DrawRectangle(x + tile_inset, y + tile_inset, tile_size - tile_inset * 2, tile_size - tile_inset * 2, color);
}

static void gameplay_loop(GameState *state) {
    const auto displayed_points_tick_time = 0.05;

    auto time = GetTime();
    auto delta_time = GetFrameTime();

    auto mouse_x = GetMouseX();
    auto mouse_y = GetMouseY();

    int mouse_tile_x;
    int mouse_tile_y;
    screen_to_tile(mouse_x, mouse_y, &mouse_tile_x, &mouse_tile_y);

    if(state->last_displayed_points_tick + displayed_points_tick_time <= time) {
        state->last_displayed_points_tick = time;

        if(state->points > state->displayed_points) {
            state->displayed_points += 1;
        } else if(state->points < state->displayed_points) {
            state->displayed_points -= 1;
        }
    }

    if(state->falling) {
        state->falling_velocity += 100.0f * delta_time;
        state->falling_amount += state->falling_velocity * delta_time;

        for(size_t i = 0; i < state->falling_tiles.count; i += 1) {
            auto tile = state->falling_tiles[i];

            auto falling_y = tile.start_y + state->falling_amount;

            if(falling_y >= tile.end_y) {
                state->tiles[tile.end_y][tile.x] = tile.kind;

                remove_at(&state->falling_tiles, i);
                i -= 1;
            }
        }

        if(state->falling_tiles.count == 0) {
            state->falling = false;
        }
    }

    for(size_t i = 0; i < state->particles.count; i += 1) {
        auto particle = &state->particles[i];

        if(particle->creation_time + particle->lifetime <= time) {
            remove_at(&state->particles, i);
            i -= 1;
        } else {
            particle->x += particle->velocity_x * delta_time;
            particle->y += particle->velocity_y * delta_time;
        }
    }

    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !state->dragging && !state->falling) {
        if(in_playfield(mouse_tile_x, mouse_tile_y)) {
            state->dragging = true;
            state->drag_start_mouse_x = mouse_x;
            state->drag_start_mouse_y = mouse_y;
            state->drag_start_tile_x = mouse_tile_x;
            state->drag_start_tile_y = mouse_tile_y;
        }
    }

    int drag_difference_x;
    int drag_difference_y;
    if(state->dragging) {
        drag_difference_x = mouse_x - state->drag_start_mouse_x;
        drag_difference_y = mouse_y - state->drag_start_mouse_y;
    }

    if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && state->dragging) {
        state->dragging = false;

        const auto fuzzy_delta = tile_size / 4;

        auto swapping = false;
        int drag_target_tile_x;
        int drag_target_tile_y;
        if(abs(drag_difference_x) > abs(drag_difference_y)) {
            if(abs(drag_difference_x) >= tile_size - fuzzy_delta) {
                swapping = true;

                if(drag_difference_x > 0) {
                    drag_target_tile_x = state->drag_start_tile_x + 1;
                } else {
                    drag_target_tile_x = state->drag_start_tile_x - 1;
                }
                drag_target_tile_y = state->drag_start_tile_y;
            }
        } else {
            if(abs(drag_difference_y) >= tile_size - fuzzy_delta) {
                swapping = true;

                drag_target_tile_x = state->drag_start_tile_x;
                if(drag_difference_y > 0) {
                    drag_target_tile_y = state->drag_start_tile_y + 1;
                } else {
                    drag_target_tile_y = state->drag_start_tile_y - 1;
                }
            }
        }

        if(swapping && in_playfield(drag_target_tile_x, drag_target_tile_y)) {
            auto from_tile_type = state->tiles[state->drag_start_tile_y][state->drag_start_tile_x];
            auto to_tile_type = state->tiles[drag_target_tile_y][drag_target_tile_x];

            state->tiles[state->drag_start_tile_y][state->drag_start_tile_x] = to_tile_type;
            state->tiles[drag_target_tile_y][drag_target_tile_x] = from_tile_type;

            bool counted[playfield_size][playfield_size] {};

            auto to_count = count_neighbours(*state, counted, drag_target_tile_x, drag_target_tile_y, from_tile_type);

            auto completed_groups = false;
            if(to_count >= 3) {
                state->points += to_count;

                delete_neighbours(state, time, drag_target_tile_x, drag_target_tile_y, from_tile_type);

                completed_groups = true;
            }

            auto from_count = count_neighbours(*state, counted, state->drag_start_tile_x, state->drag_start_tile_y, to_tile_type);

            if(from_count >= 3) {
                state->points += from_count;

                delete_neighbours(state, time, state->drag_start_tile_x, state->drag_start_tile_y, to_tile_type);

                completed_groups = true;
            }

            if(completed_groups) {
                state->falling = true;
                state->falling_tiles.count = 0;
                state->falling_velocity = 0;
                state->falling_amount = 0;

                state->last_displayed_points_tick = time;

                for(auto x = 0; x < playfield_size; x += 1) {
                    auto space_count = 0;

                    for(auto offset_y = 0; offset_y <= playfield_size - 1; offset_y += 1) {
                        auto y = playfield_size - 1 - offset_y;

                        auto kind = state->tiles[y][x];

                        if(kind == 0) {
                            space_count += 1;
                        } else if(space_count > 0) {
                            append(&state->falling_tiles, { x, y, y + space_count, kind });

                            state->tiles[y][x] = 0;
                        }
                    }

                    for(auto i = 0; i < space_count; i += 1) {
                        append(&state->falling_tiles, { x, 0 - space_count + i, i, GetRandomValue(1, tile_kind_count) });
                    }
                }
            }
        }
    }

    int drag_target_tile_x;
    int drag_target_tile_y;
    int drag_offset_screen_x;
    int drag_offset_screen_y;
    if(state->dragging) {
        bool horizontal;

        if(abs(drag_difference_x) > abs(drag_difference_y)) {
            int target_x;
            if(drag_difference_x > 0) {
                target_x = state->drag_start_tile_x + 1;

            } else {
                target_x = state->drag_start_tile_x - 1;
            }
            auto target_y = state->drag_start_tile_y;

            if(in_playfield(target_x, target_y)) {
                horizontal = true;
            } else {
                horizontal = false;
            }
        } else {
            auto target_x = state->drag_start_tile_x;
            int target_y;
            if(drag_difference_y > 0) {
                target_y = state->drag_start_tile_y + 1;
            } else {
                target_y = state->drag_start_tile_y - 1;
            }

            if(in_playfield(target_x, target_y)) {
                horizontal = false;
            } else {
                horizontal = true;
            }
        }

        if(horizontal) {
            if(drag_difference_x > 0) {
                drag_target_tile_x = state->drag_start_tile_x + 1;

            } else {
                drag_target_tile_x = state->drag_start_tile_x - 1;
            }
            drag_target_tile_y = state->drag_start_tile_y;

            drag_offset_screen_x = max(min(drag_difference_x, tile_size), -tile_size);
            drag_offset_screen_y = 0;
        } else {
            drag_target_tile_x = state->drag_start_tile_x;
            if(drag_difference_y > 0) {
                drag_target_tile_y = state->drag_start_tile_y + 1;
            } else {
                drag_target_tile_y = state->drag_start_tile_y - 1;
            }

            drag_offset_screen_x = 0;
            drag_offset_screen_y = max(min(drag_difference_y, tile_size), -tile_size);
        }
    }

    BeginDrawing();

    ClearBackground(RAYWHITE);

    for(auto y = 0; y < playfield_size; y += 1) {
        for(auto x = 0; x < playfield_size; x += 1) {
            auto tile_kind = state->tiles[y][x];

            if(tile_kind == 0) {
                continue;
            }

            int screen_x;
            int screen_y;
            tile_to_screen(x, y, &screen_x, &screen_y);

            if(state->dragging) {
                if(x == state->drag_start_tile_x && y == state->drag_start_tile_y) {
                    continue;
                } else if(x == drag_target_tile_x && y == drag_target_tile_y) {
                    continue;
                }
            }

            draw_tile_at(screen_x, screen_y, tile_kind);
        }
    }

    if(state->dragging) {
        if(in_playfield(drag_target_tile_x, drag_target_tile_y)) {
            int screen_x;
            int screen_y;
            tile_to_screen(drag_target_tile_x, drag_target_tile_y, &screen_x, &screen_y);

            screen_x -= drag_offset_screen_x;
            screen_y -= drag_offset_screen_y;

            draw_tile_at(screen_x, screen_y, state->tiles[drag_target_tile_y][drag_target_tile_x]);
        }

        {
            int screen_x;
            int screen_y;
            tile_to_screen(state->drag_start_tile_x, state->drag_start_tile_y, &screen_x, &screen_y);

            screen_x += drag_offset_screen_x;
            screen_y += drag_offset_screen_y;

            draw_tile_at(screen_x, screen_y, state->tiles[state->drag_start_tile_y][state->drag_start_tile_x]);

            DrawRectangleLinesEx({ (float)screen_x, (float)screen_y, tile_size, tile_size }, tile_inset, DARKGRAY);
        }
    }

    if(state->falling) {
        for(auto tile : state->falling_tiles) {
            int screen_x;
            int screen_y;
            tile_to_screen(tile.x, tile.start_y, &screen_x, &screen_y);

            screen_y = (int)(screen_y + state->falling_amount * tile_size);

            draw_tile_at(screen_x, screen_y, tile.kind);
        }
    }

    for(auto particle : state->particles) {
        int playfield_x;
        int playfield_y;
        playfield_position(&playfield_x, &playfield_y);

        auto x = (int)(particle.x * tile_size + playfield_x);
        auto y = (int)(particle.y * tile_size + playfield_y);

        const auto size = tile_size / 3;

        DrawRectangle(x, y, size, size, particle.color);
    }

    char buffer[128];
    snprintf(buffer, 128, "%d", state->displayed_points);

    const auto font_size = 40;

    auto text_width = MeasureText(buffer, font_size);

    DrawText(buffer, window_width / 2 - text_width / 2, 100, font_size, DARKGRAY);

    EndDrawing();
}

#if defined(PLATFORM_WEB)
GameState web_state {};

static void web_gameplay_loop() {
    gameplay_loop(&web_state);
}
#endif

int main(int argument_count, const char *arguments[]) {
    InitWindow(window_width, window_height, "Match Three");

#if defined(PLATFORM_WEB)
    auto state = &web_state;
#else
    GameState the_state {};

    auto state = &the_state;
#endif

    for(auto y = 0; y < playfield_size; y += 1) {
        for(auto x = 0; x < playfield_size; x += 1) {
            state->tiles[y][x] = GetRandomValue(1, tile_kind_count);
        }
    }

    state->last_displayed_points_tick = GetTime();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(web_gameplay_loop, 0, 1);
#else
    SetTargetFPS(60);

    while(!WindowShouldClose()) {
        gameplay_loop(state);
    }
#endif

    CloseWindow();

    return 0;
}