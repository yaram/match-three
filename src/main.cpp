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

int main(int argument_count, const char *arguments[]) {
    const auto window_width = 1280;
    const auto window_height = 720;

    InitWindow(window_width, window_height, "Match Three");

    SetTargetFPS(60);

    int tiles[playfield_size][playfield_size] {};

    for(auto y = 0; y < playfield_size; y += 1) {
        for(auto x = 0; x < playfield_size; x += 1) {
            tiles[y][x] = GetRandomValue(1, tile_kind_count);
        }
    }

    auto tile_selected = false;
    int selected_tile_x;
    int selected_tile_y;

    const auto swap_time = 0.2f;

    auto swapping = false;
    int swapping_from_x;
    int swapping_from_y;
    int swapping_to_x;
    int swapping_to_y;
    float swapping_start_time;

    const auto fall_time = 0.2f;

    auto falling = false;
    float falling_start_time;

    auto points = 0;

    while(!WindowShouldClose()) {
        const auto tile_size = 32;

        auto mouse_x = GetMouseX();
        auto mouse_y = GetMouseY();

        if(swapping) {
            auto swap_progress = (GetTime() - swapping_start_time) / swap_time;

            if(swap_progress >= 1) {
                auto from_tile_type = tiles[swapping_from_y][swapping_from_x];
                auto to_tile_type = tiles[swapping_to_y][swapping_to_x];

                tiles[swapping_from_y][swapping_from_x] = to_tile_type;
                tiles[swapping_to_y][swapping_to_x] = from_tile_type;

                bool counted[playfield_size][playfield_size] {};

                auto to_count = count_neighbours(tiles, counted, swapping_to_x, swapping_to_y, from_tile_type);

                auto completed_groups = false;
                if(to_count >= 3) {
                    points += to_count;

                    delete_neighbours(tiles, swapping_to_x, swapping_to_y, from_tile_type);

                    completed_groups = true;
                }

                auto from_count = count_neighbours(tiles, counted, swapping_from_x, swapping_from_y, to_tile_type);

                if(from_count >= 3) {
                    points += from_count;

                    delete_neighbours(tiles, swapping_from_x, swapping_from_y, to_tile_type);

                    completed_groups = true;
                }

                if(completed_groups) {
                    falling = true;
                    falling_start_time = GetTime();
                    
                }

                swapping = false;
            }
        }

        if(falling) {
            auto fall_progress = (GetTime() - falling_start_time) / fall_time;

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

        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !swapping && !falling) {
            if(
                mouse_x >= window_width / 2 - playfield_size * tile_size / 2 &&
                mouse_y >= window_height / 2 - playfield_size * tile_size / 2 &&
                mouse_x < window_width / 2 + playfield_size * tile_size / 2 &&
                mouse_y < window_height / 2 + playfield_size * tile_size / 2
            ) {
                auto relative_mouse_x = mouse_x - (window_width / 2 - playfield_size * tile_size / 2);
                auto relative_mouse_y = mouse_y - (window_height / 2 - playfield_size * tile_size / 2);

                auto tile_x = (int)((float)relative_mouse_x / tile_size);
                auto tile_y = (int)((float)relative_mouse_y / tile_size);

                if(tile_selected) {
                    if(
                        (tile_y == selected_tile_y && abs(tile_x - selected_tile_x) == 1) ||
                        (tile_x == selected_tile_x && abs(tile_y - selected_tile_y) == 1)
                    ) {
                        tile_selected = false;

                        swapping = true;
                        swapping_from_x = selected_tile_x;
                        swapping_from_y = selected_tile_y;
                        swapping_to_x = tile_x;
                        swapping_to_y = tile_y;
                        swapping_start_time = GetTime();
                    } else {
                        selected_tile_x = tile_x;
                        selected_tile_y = tile_y;
                    }
                } else {
                    tile_selected = true;
                    selected_tile_x = tile_x;
                    selected_tile_y = tile_y;
                }
            } else {
                tile_selected = false;
            }
        }

        BeginDrawing();

        ClearBackground(RAYWHITE);

        for(auto y = 0; y < playfield_size; y += 1) {
            for(auto x = 0; x < playfield_size; x += 1) {
                auto tile_kind = tiles[y][x];

                if(tile_kind != 0) {
                    Color color;
                    switch(tile_kind) {
                        case 1: color = RED; break;
                        case 2: color = GREEN; break;
                        case 3: color = BLUE; break;
                        case 4: color = YELLOW; break;
                        case 5: color = SKYBLUE; break;
                        case 6: color = PURPLE; break;
                        default: abort();
                    }

                    Rectangle rectangle {
                        window_width / 2 - playfield_size * tile_size / 2 + x * tile_size,
                        window_height / 2 - playfield_size * tile_size / 2 + y * tile_size,
                        tile_size,
                        tile_size
                    };

                    const auto inset = 2;
                    Rectangle inner_rectangle {
                        rectangle.x + inset, rectangle.y + inset,
                        rectangle.width - inset * 2, rectangle.height - inset * 2
                    };

                    if(swapping) {
                        auto swap_progress = (GetTime() - swapping_start_time) / swap_time;

                        if(x == swapping_from_x && y == swapping_from_y) {
                            inner_rectangle.x = rectangle.x + (swapping_to_x - x) * swap_progress * tile_size + inset;
                            inner_rectangle.y = rectangle.y + (swapping_to_y - y) * swap_progress * tile_size + inset;
                        } else if(x == swapping_to_x && y == swapping_to_y) {
                            inner_rectangle.x = rectangle.x + (swapping_from_x - x) * swap_progress * tile_size + inset;
                            inner_rectangle.y = rectangle.y + (swapping_from_y - y) * swap_progress * tile_size + inset;
                        }
                    }

                    DrawRectangleRec(inner_rectangle, color);

                    if(tile_selected && x == selected_tile_x && y == selected_tile_y) {
                        DrawRectangleLinesEx(rectangle, 2, DARKGRAY);
                    }
                }
            }
        }

        char buffer[128];
        snprintf(buffer, 128, "Points: %d", points);

        DrawText(buffer, 100, 100, 20, DARKGRAY);

        EndDrawing();
    }

    return 0;
}