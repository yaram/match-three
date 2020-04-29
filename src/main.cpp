#include <stdlib.h>
#include "raylib.h"

enum struct TileKind {
    None,
    Red,
    Green,
    Blue,
    Yellow,
    Cyan,
    Magenta
};

const auto playfield_size = 10;

static bool in_playfield(int x, int y) {
    return x >= 0 && y >= 0 && x < playfield_size && y < playfield_size;
}

static int count_neighbours(const TileKind tiles[playfield_size][playfield_size], bool counted[playfield_size][playfield_size], int x, int y, TileKind kind) {
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

static void delete_neighbours(TileKind tiles[playfield_size][playfield_size], int x, int y, TileKind kind) {
    tiles[y][x] = TileKind::None;

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

static void update_playfield(TileKind tiles[playfield_size][playfield_size]) {
    bool outer_done;
    do {
        bool done;
        do {
            done = true;

            for(auto reverse_y = 0; reverse_y < playfield_size; reverse_y += 1) {
                auto y = playfield_size - 1 - reverse_y;

                for(auto x = 0; x < playfield_size; x += 1) {
                    auto kind = tiles[y][x];

                    if(kind != TileKind::None) {
                        bool counted[playfield_size][playfield_size] {};

                        auto count = count_neighbours(tiles, counted, x, y, kind);

                        if(count >= 3) {
                            delete_neighbours(tiles, x, y, kind);

                            done = false;
                        }
                    }
                }
            }

            for(auto x = 0; x < playfield_size; x += 1) {
                for(auto x = 0; x < playfield_size; x += 1) {
                    for(auto offset_y = 0; offset_y < playfield_size - 1; offset_y += 1) {
                        auto y = playfield_size - 1 - offset_y;

                        if(tiles[y][x] == TileKind::None && tiles[y - 1][x] != TileKind::None) {
                            tiles[y][x] = tiles[y - 1][x];
                            tiles[y - 1][x] = TileKind::None;

                            done = false;
                        }
                    }
                }
            }
        } while(!done);

        outer_done = true;

        for(auto y = 0; y < playfield_size; y += 1) {
            for(auto x = 0; x < playfield_size; x += 1) {
                if(tiles[y][x] == TileKind::None) {
                    tiles[y][x] = (TileKind)GetRandomValue(1, 6);

                    outer_done = false;
                }
            }
        }
    } while(!outer_done);
}

int main(int argument_count, const char *arguments[]) {
    const auto window_width = 1280;
    const auto window_height = 720;

    InitWindow(window_width, window_height, "Match Three");

    SetTargetFPS(60);

    TileKind tiles[playfield_size][playfield_size] {};

    for(auto y = 0; y < playfield_size; y += 1) {
        for(auto x = 0; x < playfield_size; x += 1) {
            tiles[y][x] = (TileKind)GetRandomValue(1, 6);
        }
    }

    update_playfield(tiles);

    auto tile_selected = false;
    int selected_tile_x;
    int selected_tile_y;

    while(!WindowShouldClose()) {
        const auto tile_size = 32;

        auto mouse_x = GetMouseX();
        auto mouse_y = GetMouseY();

        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
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
                        auto selected_old = tiles[selected_tile_y][selected_tile_x];

                        tiles[selected_tile_y][selected_tile_x] = tiles[tile_y][tile_x];
                        tiles[tile_y][tile_x] = selected_old;

                        update_playfield(tiles);

                        tile_selected = false;
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

        ClearBackground(WHITE);

        for(auto y = 0; y < playfield_size; y += 1) {
            for(auto x = 0; x < playfield_size; x += 1) {
                auto tile_kind = tiles[y][x];

                if(tile_kind != TileKind::None) {
                    Color color;
                    switch(tile_kind) {
                        case TileKind::Red: color = RED; break;
                        case TileKind::Green: color = GREEN; break;
                        case TileKind::Blue: color = BLUE; break;
                        case TileKind::Yellow: color = YELLOW; break;
                        case TileKind::Cyan: color = SKYBLUE; break;
                        case TileKind::Magenta: color = PURPLE; break;
                        default: abort();
                    }

                    Rectangle rectangle {
                        window_width / 2 - playfield_size * tile_size / 2 + x * tile_size,
                        window_height / 2 - playfield_size * tile_size / 2 + y * tile_size,
                        tile_size,
                        tile_size
                    };

                    DrawRectangleRec(rectangle, color);

                    if(tile_selected && x == selected_tile_x && y == selected_tile_y) {
                        DrawRectangleLinesEx(rectangle, 2, DARKGRAY);
                    }
                }
            }
        }

        EndDrawing();
    }

    return 0;
}