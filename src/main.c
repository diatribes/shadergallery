#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "raylib.h"
#include "rlgl.h"


#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
    #define GLSL_VERSION 100
#else
    #define GLSL_VERSION 330
#endif

#define TARGET_FPS 100
#define W 320
#define H 200

enum shader_enum{
    shader_enum_plasma = 0,
    shader_enum_curtains,
    shader_enum_count,
};

Camera3D camera = { 0 };
Shader shader[shader_enum_count];
RenderTexture2D target[shader_enum_count];
Vector3 shader_frame[shader_enum_count] = {0};

extern inline float dist(float x1, float y1, float x2, float y2)
{
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

void draw_shader()
{
    for(int i = 0; i < shader_enum_count; i++) {
        BeginShaderMode(shader[i]);
            DrawBillboard(camera, target[i].texture, shader_frame[i], 2.0f, WHITE);
        EndShaderMode();
    }
}

void main_loop_body()
{
    float time = GetTime();
    float w = W;
    float h = H;

    UpdateCamera(&camera, CAMERA_FIRST_PERSON);

    if (IsKeyPressed(KEY_F)) {
        if (IsWindowFullscreen()) {
            RestoreWindow();
        } else {
            ToggleBorderlessWindowed();
        }
    }

    for (int i = 0; i < shader_enum_count; i++) {
        int timeloc = GetShaderLocation(shader[i], "time");
        int wloc = GetShaderLocation(shader[i], "W");
        int hloc = GetShaderLocation(shader[i], "H");
        SetShaderValue(shader[i], timeloc, &time, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader[i], wloc, &w, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader[i], hloc, &h, SHADER_UNIFORM_FLOAT);
    }

    BeginDrawing();

        ClearBackground(WHITE);

        BeginMode3D(camera);
            DrawGrid(60, .5f);
            draw_shader();
        EndMode3D();
        DrawFPS(10, 10);

    EndDrawing();
}

int main(int argc, char * argv[])
{

    InitWindow(W, H, "GPU Plasma");
    
    shader[shader_enum_plasma] = LoadShader(0, TextFormat("resources/plasma_%i.fs", GLSL_VERSION));
    shader[shader_enum_curtains] = LoadShader(0, TextFormat("resources/curtains_%i.fs", GLSL_VERSION));
    target[shader_enum_plasma] = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    target[shader_enum_curtains] = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

    camera.position = (Vector3){ -7.f, 1.0f, -9.f };    // Camera position
    camera.target = (Vector3){ 2.4f, 3.0f, 0.0f };    // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type
                                                        //
    shader_frame[shader_enum_plasma].x = 0.f;
    shader_frame[shader_enum_plasma].y = 1.2f;
    shader_frame[shader_enum_plasma].z = 0.f;

    shader_frame[shader_enum_curtains].x = -5.0f;
    shader_frame[shader_enum_curtains].y = 1.2f;
    shader_frame[shader_enum_curtains].z = 0.f;

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(main_loop_body, 120, 1);
#else

    SetTargetFPS(TARGET_FPS);
    DisableCursor();
    ToggleBorderlessWindowed();
    while (!WindowShouldClose()) {
        main_loop_body();
    }
    EnableCursor();
#endif

    CloseWindow();
    return 0;
}

