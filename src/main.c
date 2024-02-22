#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

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

#define ROOM_MAX_COLUMNS 4
#define ROOM_MAX_KNOTS 2

#define COLUMN_SIZE .25
#define COLUMN_HEIGHT .75
#define WALL_HEIGHT 8.0

struct column {
    Model model;
    Vector3 pos;
};
struct knot {
    Model model;
    Vector3 pos;
};
struct column column[ROOM_MAX_COLUMNS];
struct knot knot[ROOM_MAX_KNOTS];
Model floor_model;
Model ceiling_model;
Model walls[4];

enum shader_enum {
    shader_enum_plasma = 0,
    shader_enum_plasma_squares,
    shader_enum_plasma_disco,
    shader_enum_curtains,
    shader_enum_lights,
    shader_enum_count,
};

Camera3D camera = { 0 };
Shader shader[shader_enum_count];
RenderTexture2D target[shader_enum_count];
Vector3 shader_frame[shader_enum_count] = {0};

Light lights[MAX_LIGHTS] = { 0 };

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

void draw_gallery()
{
    // Draw cylinders
    for (int i = 0; i < ROOM_MAX_COLUMNS; i++) {
        DrawModel(column[i].model, column[i].pos, 1.0, GRAY);
    }

    // Draw knots 
    for (int i = 0; i < ROOM_MAX_KNOTS; i++) {
        DrawModel(knot[i].model, knot[i].pos, 1.0, RED);
    }

    // Draw walls 
    int y = WALL_HEIGHT;
    DrawModel(walls[0], (Vector3) { -16, y / 2, 0.0 }, 1.0, DARKGRAY);
    DrawModel(walls[1], (Vector3) { 16, y / 2, 0.0 }, 1.0, DARKGRAY);
    DrawModel(walls[2], (Vector3) { 0, y / 2, 16.0 }, 1.0, DARKGRAY);
    DrawModel(walls[3], (Vector3) { 0, y / 2, -16.0 }, 1.0, DARKGRAY);

    // Draw floor and ceiling 
    DrawModel(floor_model, (Vector3) { 0,0,0 }, 1.0, WHITE);
    DrawModel(ceiling_model, (Vector3) { 0,WALL_HEIGHT,0 }, 1.0, GRAY);

}

void draw_shader_frames()
{
    // Draw plasma shaders 
    for(int i = 0; i < shader_enum_count; i++) {

        Vector2 size = {1.0, 1.0};
        Vector2 origin = {0, 0};
        Vector3 up = (Vector3) { 0.0f, 1.0f, 0.0f };

        rlDisableBackfaceCulling();
        //rlDisableDepthMask();
        BeginShaderMode(shader[i]);

            draw_shader_frame(camera, target[i].texture, shader_frame[i], up, size, origin, WHITE);

        EndShaderMode();
        rlEnableBackfaceCulling();
        //rlEnableDepthMask();
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

    // TODO: Lights don't match up with key numbers
    if (IsKeyPressed(KEY_ONE)) {
        lights[0].enabled ^= 1;
    }
    if (IsKeyPressed(KEY_TWO)) {
        lights[1].enabled ^= 1;
    }
    if (IsKeyPressed(KEY_THREE)) {
        lights[2].enabled ^= 1;
    }
    if (IsKeyPressed(KEY_FOUR)) {
        lights[3].enabled ^= 1;
    }

    // Update camera and pass new position to light shader
    UpdateCamera(&camera, CAMERA_FIRST_PERSON);
    SetShaderValue(shader[shader_enum_lights], shader[shader_enum_lights].locs[SHADER_LOC_VECTOR_VIEW],
            (float[3]) { camera.position.x, camera.position.y, camera.position.z }, SHADER_UNIFORM_VEC3);
    for (int i = 0; i < MAX_LIGHTS; i++) {
        UpdateLightValues(shader[shader_enum_lights], lights[i]);
    }

    // Update width, height, and time on the other shaders
    for (int i = 0; i < shader_enum_count; i++) {
        if (i == shader_enum_lights) {
            /*
             * Do nothing
             */
        } else {
            int time_loc = GetShaderLocation(shader[i], "time");
            int w_loc = GetShaderLocation(shader[i], "W");
            int h_loc = GetShaderLocation(shader[i], "H");
            SetShaderValue(shader[i], time_loc, &time, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shader[i], w_loc, &w, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shader[i], h_loc, &h, SHADER_UNIFORM_FLOAT);
        }
    }

    BeginDrawing();

        ClearBackground(WHITE);

        BeginMode3D(camera);

            draw_gallery();
            draw_shader_frames();

        EndMode3D();

        DrawText(TextFormat("x:%f y:%f z:%f", camera.position.x, camera.position.y, camera.position.z), 10, 30, 20, GREEN);
        DrawFPS(10, 10);

    EndDrawing();
}

void init()
{
    // Light shader
    shader[shader_enum_lights] = LoadShader(TextFormat("resources/lights_%i.vs", GLSL_VERSION),
        TextFormat("resources/lights_%i.fs", GLSL_VERSION));

    // Load plasma shaders
    shader[shader_enum_plasma] = LoadShader(0, TextFormat("resources/plasma_%i.fs", GLSL_VERSION));
    shader[shader_enum_plasma_squares] = LoadShader(0, TextFormat("resources/plasma_squares_%i.fs", GLSL_VERSION));
    shader[shader_enum_plasma_disco] = LoadShader(0, TextFormat("resources/plasma_disco_%i.fs", GLSL_VERSION));
    shader[shader_enum_curtains] = LoadShader(0, TextFormat("resources/curtains_%i.fs", GLSL_VERSION));

    // Create render surface for shaders
    target[shader_enum_plasma] = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    target[shader_enum_plasma_squares] = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    target[shader_enum_plasma_disco] = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    target[shader_enum_curtains] = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

    // Init camera
    camera.position = (Vector3) { -7.f, 1.5f, -9.f };
    camera.target = (Vector3) { 2.4f, .5f, -1.0f };
    camera.up = (Vector3) { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
   
    // Create lights
    lights[0] = CreateLight(LIGHT_POINT, (Vector3) { -4.0, COLUMN_HEIGHT * 2,15.0 },
            Vector3Zero(), GREEN,  shader[shader_enum_lights]);
    lights[1] = CreateLight(LIGHT_POINT, (Vector3) { 0.0, COLUMN_HEIGHT * 2, 15.0 },
            Vector3Zero(), RED, shader[shader_enum_lights]);
    lights[2] = CreateLight(LIGHT_POINT, (Vector3) { 4.0, COLUMN_HEIGHT * 2, 15.0 },
            Vector3Zero(), BLUE, shader[shader_enum_lights]);
    lights[3] = CreateLight(LIGHT_POINT, (Vector3) { 8.0, COLUMN_HEIGHT * 2, 15.0 },
            Vector3Zero(), WHITE, shader[shader_enum_lights]);
    lights[0].enabled = true;
    lights[1].enabled = true;
    lights[2].enabled = true;
    lights[3].enabled = true;

    // Set the ambient light values
    int ambient_loc = GetShaderLocation(shader[shader_enum_lights], "ambient");
    SetShaderValue(shader[shader_enum_lights], ambient_loc, (float[4]) { .1, .1, .1, 1 }, SHADER_UNIFORM_VEC4);
    shader[shader_enum_lights].locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader[shader_enum_lights], "viewPos");

    // Load and config floor texture
    Texture floor_texture = LoadTexture("resources/floor.png");
    GenTextureMipmaps(&floor_texture);
    SetTextureFilter(floor_texture, RL_TEXTURE_FILTER_TRILINEAR);
    SetTextureWrap(floor_texture, TEXTURE_WRAP_REPEAT);

    // Load and config ceiling texture
    Texture ceiling_texture = LoadTexture("resources/ceiling.png");
    GenTextureMipmaps(&ceiling_texture);
    SetTextureFilter(ceiling_texture, RL_TEXTURE_FILTER_TRILINEAR);
    SetTextureWrap(ceiling_texture, TEXTURE_WRAP_REPEAT);

    // Load and config column texture
    Texture column_texture = LoadTexture("resources/column.png");
    GenTextureMipmaps(&column_texture);
    SetTextureFilter(column_texture, RL_TEXTURE_FILTER_TRILINEAR);
    SetTextureWrap(column_texture, TEXTURE_WRAP_REPEAT);

    // Load and config wall texture
    Texture wall_texture = LoadTexture("resources/wall.png");
    GenTextureMipmaps(&wall_texture);
    SetTextureFilter(wall_texture, RL_TEXTURE_FILTER_TRILINEAR);
    SetTextureWrap(wall_texture, TEXTURE_WRAP_REPEAT);

    // Generate the knot mesh
    Mesh knot_mesh = GenMeshKnot(1.0, 3.0, 128, 128);
    for (int i = 0; i < knot_mesh.vertexCount * 2; i++) {
        // Stretch texcoords to wrap texture
        knot_mesh.texcoords[i] *= .25;
    }
    UpdateMeshBuffer(knot_mesh, 1, knot_mesh.texcoords, knot_mesh.vertexCount * 2 * sizeof(float), 0);

    // Generate the cylinder mesh
    Mesh cylinder_mesh = GenMeshCylinder(COLUMN_SIZE, COLUMN_HEIGHT, 32);
    for (int i = 0; i < cylinder_mesh.vertexCount * 2; i++) {
        // Stretch texcoords to wrap texture
        cylinder_mesh.texcoords[i] *= 4.25;
    }
    UpdateMeshBuffer(cylinder_mesh, 1, cylinder_mesh.texcoords, cylinder_mesh.vertexCount * 2 * sizeof(float), 0);

    // Generate the floor mesh
    Mesh floor_mesh = GenMeshPlane(32.0f, 32.0f, 16, 16);
    for (int i = 0; i < floor_mesh.vertexCount * 2; i++) {
        // Stretch texcoords to wrap texture
        floor_mesh.texcoords[i] *= 4.25;
    }
    UpdateMeshBuffer(floor_mesh, 1, floor_mesh.texcoords, floor_mesh.vertexCount * 2 * sizeof(float), 0);

    // Generate the ceiling mesh
    Mesh ceiling_mesh = GenMeshCube(32, 1, 32);
    for (int i = 0; i < ceiling_mesh.vertexCount * 2; i++) {
        // Stretch texcoords to wrap texture
        ceiling_mesh.texcoords[i] *= 6.25;
    }
    UpdateMeshBuffer(ceiling_mesh, 1, ceiling_mesh.texcoords, ceiling_mesh.vertexCount * 2 * sizeof(float), 0);

    // Create floor model from mesh
    floor_model = LoadModelFromMesh(floor_mesh);
    floor_model.materials[0].shader = shader[shader_enum_lights];
    floor_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = floor_texture;

    // Create ceiling model from mesh
    ceiling_model = LoadModelFromMesh(ceiling_mesh);
    ceiling_model.materials[0].shader = shader[shader_enum_lights];
    ceiling_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = ceiling_texture;

    // Create wall models from mesh
    walls[0] = LoadModelFromMesh(GenMeshCube(1.0, WALL_HEIGHT, 32.0f));
    walls[1] = LoadModelFromMesh(GenMeshCube(1.0, WALL_HEIGHT, 32.0f));
    walls[2] = LoadModelFromMesh(GenMeshCube(32.0, WALL_HEIGHT, 1.0f));
    walls[3] = LoadModelFromMesh(GenMeshCube(32.0, WALL_HEIGHT, 1.0f));
    for (int i = 0; i < sizeof(walls) / sizeof(walls[0]); i++) {
        for (int j = 0; j < walls[i].meshes[0].vertexCount * 2; j++) {
            // Stretch texcoords to wrap texture
            walls[i].meshes[0].texcoords[j] *= 16.25;
        }
        UpdateMeshBuffer(walls[i].meshes[0], 1, walls[i].meshes[0].texcoords, walls[i].meshes[0].vertexCount * 2 * sizeof(float), 0); // Update mesh vertex data in GPU for a specific buffer index
        walls[i].materials[0].shader = shader[shader_enum_lights];
        walls[0].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = wall_texture;
    }

    // Set wall texture
    walls[1].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = wall_texture;
    walls[2].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = wall_texture;
    walls[3].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = wall_texture;

    // Position and config columns
    column[0].pos.x = -4.0;
    column[0].pos.y = 0.0;
    column[0].pos.z = 14.0;
    column[0].model = LoadModelFromMesh(cylinder_mesh);
    column[0].model.materials[0].shader = shader[shader_enum_lights];
    column[0].model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = column_texture;

    column[1].pos.x = 0.0;
    column[1].pos.y = 0.0;
    column[1].pos.z = 14.0;
    column[1].model = LoadModelFromMesh(cylinder_mesh);
    column[1].model.materials[0].shader = shader[shader_enum_lights];
    column[1].model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = column_texture;

    column[2].pos.x = 4.0;
    column[2].pos.y = 0.0;
    column[2].pos.z = 14.0;
    column[2].model = LoadModelFromMesh(cylinder_mesh);
    column[2].model.materials[0].shader = shader[shader_enum_lights];
    column[2].model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = column_texture;

    column[3].pos.x = 8.0;
    column[3].pos.y = 0.0;
    column[3].pos.z = 14.0;
    column[3].model = LoadModelFromMesh(cylinder_mesh);
    column[3].model.materials[0].shader = shader[shader_enum_lights];
    column[3].model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = column_texture;

    // Position and config knots
    knot[0].pos.x = -4.0;
    knot[0].pos.y = 3.0;
    knot[0].pos.z = -12;
    knot[0].model = LoadModelFromMesh(knot_mesh);
    knot[0].model.materials[0].shader = shader[shader_enum_plasma];

    knot[1].pos.x = 4.0;
    knot[1].pos.y = 3.0;
    knot[1].pos.z = -12;
    knot[1].model = LoadModelFromMesh(knot_mesh);
    knot[1].model.materials[0].shader = shader[shader_enum_plasma_squares];

    // Position and config shader surfaces 
    shader_frame[shader_enum_curtains].x = -4.0f;
    shader_frame[shader_enum_curtains].y = COLUMN_HEIGHT * 2;
    shader_frame[shader_enum_curtains].z = 14.f;

    shader_frame[shader_enum_plasma_squares].x = 0.0f;
    shader_frame[shader_enum_plasma_squares].y = COLUMN_HEIGHT * 2;
    shader_frame[shader_enum_plasma_squares].z = 14.f;

    shader_frame[shader_enum_plasma_disco].x = 4.0f;
    shader_frame[shader_enum_plasma_disco].y = COLUMN_HEIGHT * 2;
    shader_frame[shader_enum_plasma_disco].z = 14.f;

    shader_frame[shader_enum_plasma].x = 8.f;
    shader_frame[shader_enum_plasma].y = COLUMN_HEIGHT * 2;
    shader_frame[shader_enum_plasma].z = 14.f;

}

int main(int argc, char * argv[])
{
    InitWindow(W, H, "Shader Gallery");
    init();

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
