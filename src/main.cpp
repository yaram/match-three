#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "raylib.h"

const int tile_kind_count = 6;

const auto playfield_size = 10;

static bool in_playfield(int x, int y) {
    return x >= 0 && y >= 0 && x < playfield_size && y < playfield_size;
}

static int count_neighbours(const int tiles[playfield_size][playfield_size], bool counted[playfield_size][playfield_size], int x, int y, int kind) {
    counted[y][x] = true;

    auto total = 1;

    if(in_playfield(x + 1, y) && tiles[y][x + 1] == kind && !counted[y][x + 1]) {
        total += count_neighbours(tiles, counted, x + 1, y, kind);
    }

    if(in_playfield(x, y + 1) && tiles[y + 1][x] == kind && !counted[y + 1][x]) {
        total += count_neighbours(tiles, counted, x, y + 1, kind);
    }

    if(in_playfield(x - 1, y) && tiles[y][x - 1] == kind && !counted[y][x - 1]) {
        total += count_neighbours(tiles, counted, x - 1, y, kind);
    }

    if(in_playfield(x, y - 1) && tiles[y - 1][x] == kind && !counted[y - 1][x]) {
        total += count_neighbours(tiles, counted, x, y - 1, kind);
    }

    return total;
}

static void delete_neighbours(int tiles[playfield_size][playfield_size], int x, int y, int kind) {
    tiles[y][x] = 0;

    if(in_playfield(x + 1, y) && tiles[y][x + 1] == kind) {
        delete_neighbours(tiles, x + 1, y, kind);
    }

    if(in_playfield(x, y + 1) && tiles[y + 1][x] == kind) {
        delete_neighbours(tiles, x, y + 1, kind);
    }

    if(in_playfield(x - 1, y) && tiles[y][x - 1] == kind) {
        delete_neighbours(tiles, x - 1, y, kind);
    }

    if(in_playfield(x, y - 1) && tiles[y - 1][x] == kind) {
        delete_neighbours(tiles, x, y - 1, kind);
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

const auto window_width = 1280;
const auto window_height = 720;

const auto tile_size = 32;

static void screen_to_tile(int x, int y, int *tile_x, int *tile_y) {
    auto relative_x = x - window_width / 2 + playfield_size * tile_size / 2;
    auto relative_y = y - window_height / 2 + playfield_size * tile_size / 2;

    *tile_x = (int)floorf((float)relative_x / tile_size);
    *tile_y = (int)floorf((float)relative_y / tile_size);
}

static void tile_to_screen(int tile_x, int tile_y, int *x, int *y) {
    auto relative_x = tile_x * tile_size;
    auto relative_y = tile_y * tile_size;

    *x = window_width / 2 - playfield_size * tile_size / 2 + relative_x;
    *y = window_height / 2 - playfield_size * tile_size / 2 + relative_y;
}

const auto tile_inset = 2;

static void draw_tile_at(int x, int y, int kind) {
    Color color;
    switch(kind) {
        case 1: color = RED; break;
        case 2: color = GREEN; break;
        case 3: color = BLUE; break;
        case 4: color = GOLD; break;
        case 5: color = SKYBLUE; break;
        case 6: color = PURPLE; break;
        default: abort();
    }

    DrawRectangle(x + tile_inset, y + tile_inset, tile_size - tile_inset * 2, tile_size - tile_inset * 2, color);
}

int main(int argument_count, const char *arguments[]) {
    InitWindow(window_width, window_height, "Match Three");

    SetTargetFPS(60);

    int tiles[playfield_size][playfield_size] {};

    for(auto y = 0; y < playfield_size; y += 1) {
        for(auto x = 0; x < playfield_size; x += 1) {
            tiles[y][x] = GetRandomValue(1, tile_kind_count);
        }
    }

    auto dragging = false;
    int drag_start_mouse_x;
    int drag_start_mouse_y;
    int drag_start_tile_x;
    int drag_start_tile_y;

    const auto fall_time = 0.2f;

    auto falling = false;
    float falling_start_time;

    auto points = 0;

    while(!WindowShouldClose()) {
        auto time = GetTime();

        auto mouse_x = GetMouseX();
        auto mouse_y = GetMouseY();

        int mouse_tile_x;
        int mouse_tile_y;
        screen_to_tile(mouse_x, mouse_y, &mouse_tile_x, &mouse_tile_y);

        if(falling) {
            auto fall_progress = (time - falling_start_time) / fall_time;

            if(fall_progress >= 1) {
                bool done;
                do {
                    done = true;

                    for(auto x = 0; x < playfield_size; x += 1) {
                        for(auto offset_y = 0; offset_y < playfield_size - 1; offset_y += 1) {
                            auto y = playfield_size - 1 - offset_y;

                            if(tiles[y][x] == 0 && tiles[y - 1][x] != 0) {
                                tiles[y][x] = tiles[y - 1][x];
                                tiles[y - 1][x] = 0;

                                done = false;
                            }
                        }
                    }
                } while(!done);

                for(auto y = 0; y < playfield_size; y += 1) {
                    for(auto x = 0; x < playfield_size; x += 1) {
                        if(tiles[y][x] == 0) {
                            tiles[y][x] = GetRandomValue(1, tile_kind_count);
                        }
                    }
                }

                falling = false;
            }
        }

        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !dragging && !falling) {
            if(in_playfield(mouse_tile_x, mouse_tile_y)) {
                dragging = true;
                drag_start_mouse_x = mouse_x;
                drag_start_mouse_y = mouse_y;
                drag_start_tile_x = mouse_tile_x;
                drag_start_tile_y = mouse_tile_y;
            }
        }

        int drag_difference_x;
        int drag_difference_y;
        if(dragging) {
            drag_difference_x = mouse_x - drag_start_mouse_x;
            drag_difference_y = mouse_y - drag_start_mouse_y;
        }

        if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && dragging) {
            dragging = false;

            const auto fuzzy_delta = tile_size / 4;

            auto swapping = false;
            int drag_target_tile_x;
            int drag_target_tile_y;
            if(abs(drag_difference_x) > abs(drag_difference_y)) {
                if(abs(drag_difference_x) >= tile_size - fuzzy_delta) {
                    swapping = true;

                    if(drag_difference_x > 0) {
                        drag_target_tile_x = drag_start_tile_x + 1;
                    } else {
                        drag_target_tile_x = drag_start_tile_x - 1;
                    }
                    drag_target_tile_y = drag_start_tile_y;
                }
            } else {
                if(abs(drag_difference_y) >= tile_size - fuzzy_delta) {
                    swapping = true;

                    drag_target_tile_x = drag_start_tile_x;
                    if(drag_difference_y > 0) {
                        drag_target_tile_y = drag_start_tile_y + 1;
                    } else {
                        drag_target_tile_y = drag_start_tile_y - 1;
                    }
                }
            }

            if(swapping && in_playfield(drag_target_tile_x, drag_target_tile_y)) {
                auto from_tile_type = tiles[drag_start_tile_y][drag_start_tile_x];
                auto to_tile_type = tiles[drag_target_tile_y][drag_target_tile_x];

                tiles[drag_start_tile_y][drag_start_tile_x] = to_tile_type;
                tiles[drag_target_tile_y][drag_target_tile_x] = from_tile_type;

                bool counted[playfield_size][playfield_size] {};

                auto to_count = count_neighbours(tiles, counted, drag_target_tile_x, drag_target_tile_y, from_tile_type);

                auto completed_groups = false;
                if(to_count >= 3) {
                    points += to_count;

                    delete_neighbours(tiles, drag_target_tile_x, drag_target_tile_y, from_tile_type);

                    completed_groups = true;
                }

                auto from_count = count_neighbours(tiles, counted, drag_start_tile_x, drag_start_tile_y, to_tile_type);

                if(from_count >= 3) {
                    points += from_count;

                    delete_neighbours(tiles, drag_start_tile_x, drag_start_tile_y, to_tile_type);

                    completed_groups = true;
                }

                if(completed_groups) {
                    falling = true;
                    falling_start_time = time;
                }
            }
        }

        int drag_target_tile_x;
        int drag_target_tile_y;
        int drag_offset_screen_x;
        int drag_offset_screen_y;
        if(dragging) {
            if(abs(drag_difference_x) > abs(drag_difference_y)) {
                if(drag_difference_x > 0) {
                    drag_target_tile_x = drag_start_tile_x + 1;

                } else {
                    drag_target_tile_x = drag_start_tile_x - 1;
                }
                drag_target_tile_y = drag_start_tile_y;

                if(in_playfield(drag_target_tile_x, drag_target_tile_y)) {
                    drag_offset_screen_x = max(min(drag_difference_x, tile_size), -tile_size);
                    drag_offset_screen_y = 0;
                } else {
                    drag_offset_screen_x = 0;
                    drag_offset_screen_y = 0;
                }
            } else {
                drag_target_tile_x = drag_start_tile_x;
                if(drag_difference_y > 0) {
                    drag_target_tile_y = drag_start_tile_y + 1;
                } else {
                    drag_target_tile_y = drag_start_tile_y - 1;
                }

                if(in_playfield(drag_target_tile_x, drag_target_tile_y)) {
                    drag_offset_screen_x = 0;
                    drag_offset_screen_y = max(min(drag_difference_y, tile_size), -tile_size);
                } else {
                    drag_offset_screen_x = 0;
                    drag_offset_screen_y = 0;
                }
            }
        }

        BeginDrawing();

        ClearBackground(RAYWHITE);

        for(auto y = 0; y < playfield_size; y += 1) {
            for(auto x = 0; x < playfield_size; x += 1) {
                auto tile_kind = tiles[y][x];

                if(tile_kind == 0) {
                    continue;
                }

                int screen_x;
                int screen_y;
                tile_to_screen(x, y, &screen_x, &screen_y);

                if(dragging) {
                    if(x == drag_start_tile_x && y == drag_start_tile_y) {
                        continue;
                    } else if(x == drag_target_tile_x && y == drag_target_tile_y) {
                        continue;
                    }
                }

                draw_tile_at(screen_x, screen_y, tile_kind);
            }
        }

        if(dragging) {
            if(in_playfield(drag_target_tile_x, drag_target_tile_y)) {
                int screen_x;
                int screen_y;
                tile_to_screen(drag_target_tile_x, drag_target_tile_y, &screen_x, &screen_y);

                screen_x -= drag_offset_screen_x;
                screen_y -= drag_offset_screen_y;

                draw_tile_at(screen_x, screen_y, tiles[drag_target_tile_y][drag_target_tile_x]);
            }

            if(in_playfield(drag_start_tile_x, drag_start_tile_y)) {
                int screen_x;
                int screen_y;
                tile_to_screen(drag_start_tile_x, drag_start_tile_y, &screen_x, &screen_y);

                screen_x += drag_offset_screen_x;
                screen_y += drag_offset_screen_y;

                draw_tile_at(screen_x, screen_y, tiles[drag_start_tile_y][drag_start_tile_x]);

                DrawRectangleLinesEx({ (float)screen_x, (float)screen_y, tile_size, tile_size }, tile_inset, DARKGRAY);
            }
        }

        char buffer[128];
        snprintf(buffer, 128, "Points: %d", points);

        DrawText(buffer, 100, 100, 20, DARKGRAY);

        EndDrawing();
    }

    return 0;
}