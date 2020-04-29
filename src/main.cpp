#include <stdlib.h>
#include "raylib.h"

int main(int argument_count, const char *arguments[]) {
    const auto window_width = 1280;
    const auto window_height = 720;

    InitWindow(window_width, window_height, "Match Three");

    SetTargetFPS(60);

    enum struct TileKind {
        None,
        Red,
        Green,
        Blue
    };

    const size_t playfield_size = 10;

    TileKind tiles[playfield_size][playfield_size] {};

    for(size_t y = 0; y < playfield_size; y += 1) {
        for(size_t x = 0; x < playfield_size; x += 1) {
            tiles[y][x] = (TileKind)GetRandomValue(0, 3);
        }
    }

    while(!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(WHITE);

        for(size_t y = 0; y < playfield_size; y += 1) {
            for(size_t x = 0; x < playfield_size; x += 1) {
                auto tile_kind = tiles[y][x];

                if(tile_kind != TileKind::None) {
                    Color color;
                    switch(tile_kind) {
                        case TileKind::Red: color = RED; break;
                        case TileKind::Green: color = GREEN; break;
                        case TileKind::Blue: color = BLUE; break;
                        default: abort();
                    }

                    const int tile_size = 32;

                    DrawRectangle(
                        window_width / 2 - playfield_size * tile_size / 2 + x * tile_size,
                        window_height / 2 - playfield_size * tile_size / 2 + y * tile_size,
                        tile_size,
                        tile_size,
                        color
                    );
                }
            }
        }

        EndDrawing();
    }

    return 0;
}