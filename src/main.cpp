#include "raylib.h"

int main(int argument_count, const char *arguments[]) {
    const auto window_width = 1280;
    const auto window_height = 720;

    InitWindow(window_width, window_height, "Match Three");

    SetTargetFPS(60);

    while(!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(BLACK);

        DrawText("Test", 100, 100, 20, RAYWHITE);

        EndDrawing();
    }

    return 0;
}