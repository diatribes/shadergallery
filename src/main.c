#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"        // Required for: Vector3, Quaternion and Matrix functionality


#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
    #include <emscripten/html5.h>
    #define GLSL_VERSION 100
#else
    #define GLSL_VERSION 330
#endif

#define TARGET_FPS 100
#define W 800
#define H 450

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

/*
 * Adapted from rmodels.c: DrawBillboardPro()
 */
void draw_shader_frame(Camera camera, Texture2D texture, Vector3 position, Vector3 up, Vector2 size, Vector2 origin, Color tint)
{
    Rectangle source = { 0, 0, texture.width, texture.height };
    Vector2 sizeRatio = { size.x*fabsf((float)source.width/source.height), size.y };

    Vector3 right = { 1.0, 0, 0};
    Vector3 rightScaled = Vector3Scale(right, sizeRatio.x/2);
    Vector3 upScaled = Vector3Scale(up, sizeRatio.y/2);

    Vector3 p1 = Vector3Add(rightScaled, upScaled);
    Vector3 p2 = Vector3Subtract(rightScaled, upScaled);

    Vector3 topLeft = Vector3Scale(p2, -1);
    Vector3 topRight = p1;
    Vector3 bottomRight = p2;
    Vector3 bottomLeft = Vector3Scale(p1, -1);

    topLeft = Vector3Add(topLeft, position);
    topRight = Vector3Add(topRight, position);
    bottomRight = Vector3Add(bottomRight, position);
    bottomLeft = Vector3Add(bottomLeft, position);

    rlSetTexture(texture.id);

    rlBegin(RL_QUADS);
        rlColor4ub(tint.r, tint.g, tint.b, tint.a);

        if (sizeRatio.x * sizeRatio.y >= 0.0f)
        {
            // Bottom-left corner for texture and quad
            rlTexCoord2f((float)source.x/texture.width, (float)source.y/texture.height);
            rlVertex3f(topLeft.x, topLeft.y, topLeft.z);

            // Top-left corner for texture and quad
            rlTexCoord2f((float)source.x/texture.width, (float)(source.y + source.height)/texture.height);
            rlVertex3f(bottomLeft.x, bottomLeft.y, bottomLeft.z);

            // Top-right corner for texture and quad
            rlTexCoord2f((float)(source.x + source.width)/texture.width, (float)(source.y + source.height)/texture.height);
            rlVertex3f(bottomRight.x, bottomRight.y, bottomRight.z);

            // Bottom-right corner for texture and quad
            rlTexCoord2f((float)(source.x + source.width)/texture.width, (float)source.y/texture.height);
            rlVertex3f(topRight.x, topRight.y, topRight.z);
        }
        else
        {
            // Reverse vertex order if the size has only one negative dimension
            rlTexCoord2f((float)(source.x + source.width)/texture.width, (float)source.y/texture.height);
            rlVertex3f(topRight.x, topRight.y, topRight.z);

            rlTexCoord2f((float)(source.x + source.width)/texture.width, (float)(source.y + source.height)/texture.height);
            rlVertex3f(bottomRight.x, bottomRight.y, bottomRight.z);

            rlTexCoord2f((float)source.x/texture.width, (float)(source.y + source.height)/texture.height);
            rlVertex3f(bottomLeft.x, bottomLeft.y, bottomLeft.z);

            rlTexCoord2f((float)source.x/texture.width, (float)source.y/texture.height);
            rlVertex3f(topLeft.x, topLeft.y, topLeft.z);
        }

    rlEnd();

    rlSetTexture(0);
}

void draw_shader_frames()
{

    for(int i = 0; i < shader_enum_count; i++) {

        Vector2 size = {1.0, 1.0};
        Vector2 origin = {0, 0};
        Vector3 up = (Vector3){ 0.0f, 1.0f, 0.0f };

        rlDisableBackfaceCulling();
        rlDisableDepthMask();
        BeginShaderMode(shader[i]);
            draw_shader_frame(camera, target[i].texture, shader_frame[i], up, size, origin, WHITE);
        EndShaderMode();
        rlEnableBackfaceCulling();
        rlEnableDepthMask();
    }
}

void main_loop_body()
{
    float time = GetTime();
    float w = W;
    float h = H;

#if defined(PLATFORM_WEB)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        emscripten_request_pointerlock("#canvas", 0);
    }
#endif

    if (IsKeyPressed(KEY_F)) {
#if defined(PLATFORM_WEB)
        ToggleBorderlessWindowed();
#else
        ToggleBorderlessWindowed();
#endif
    }

    UpdateCamera(&camera, CAMERA_FIRST_PERSON);

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
            DrawGrid(60, 1.0f);
            draw_shader_frames();
        EndMode3D();
        DrawFPS(10, 10);

    EndDrawing();
}

int main(int argc, char * argv[])
{

    InitWindow(W, H, "Shader Gallery");
    
    shader[shader_enum_plasma] = LoadShader(0, TextFormat("resources/plasma_%i.fs", GLSL_VERSION));
    shader[shader_enum_curtains] = LoadShader(0, TextFormat("resources/curtains_%i.fs", GLSL_VERSION));
    target[shader_enum_plasma] = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    target[shader_enum_curtains] = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

    camera.position = (Vector3){ -7.f, 1.0f, -9.f };    // Camera position
    camera.target = (Vector3){ 2.4f, .5f, -1.0f };    // Camera looking at point
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
    emscripten_request_pointerlock("#canvas", 1);
    emscripten_set_main_loop(main_loop_body, 120, 1);
#else
    SetTargetFPS(TARGET_FPS);
    DisableCursor();
    ToggleBorderlessWindowed();
    while (!WindowShouldClose()) {
        main_loop_body();
    }
#endif

    CloseWindow();
    return 0;
}

