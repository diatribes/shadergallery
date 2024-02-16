#version 330 

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Custom inputs
uniform float W;
uniform float H;
uniform float time;

// Output fragment color
out vec4 finalColor;

float dist(float x1, float y1, float x2, float y2)
{
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

vec3 curtains()
{
    float t = time;
    float x = fragTexCoord.x;
    float y = fragTexCoord.y;
    float u = x * 8.0;
    float v = y * 8.0;

    float r = sin(sin(t) + .9 + cos(t + sin(u) + cos(v)));
    float g = sin(t + .7 + cos(v + u));
    float b = sin(t + .5 + cos(u) + sin(v)) * sin(u * 20.0 * sin(u + v + t)) / W;

    r += dist(x, y, r, g);
    g += 1.0/ dist(x, y, g, r) + (sin(t / 70.0) * .5 + .5) / 10.0;
    b += dist(x, y, r, g) + (sin(t / 170.0) * .5 + .5) / 10.0;

    r /= 4.0;
    g /= 4.0;
    b /= 2.0;

    return vec3 (r, g, b);


/*
    float x = fragTexCoord.x / W;
    float y = fragTexCoord.y / H;
    float u = fragTexCoord.x * 10.0;
    float v = fragTexCoord.y * 10.0;

    float r = sin(t + 9.0 + sin(t + sin(u) + cos(v)));
    float g = sin(t + 7.0 + sin(u) + cos(v));
    float b = sin(t + 5.0 + sin(u) + cos(v));

    r += dist(x, y, r, g);
    g += dist(x, y, g, r);
    b += dist(x, y, g, b);

    return vec3 (r, g, b);
*/
}

void main()
{
    finalColor = vec4(curtains(), 1.0);
}
