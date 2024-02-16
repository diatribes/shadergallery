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

vec4 plasma()
{
    float value = 0;
    float t = time / 60.f;

    float x = fragTexCoord.x / W;
    float y = fragTexCoord.y / H;

    float d1 = dist(x, y, .3, .8) * W * 12.f;
    float d2 = dist(x, y, .8, .1) * W * 11.f;
    float d3 = dist(x, y, .1, .8) * W * 10.f;
    float d4 = dist(x, y, t / 100.f, t / 100.f)* W * 11.5f;
    value = 0.5 + 0.5 * sin(d1) + sin(d2) + sin(d3) + sin(d4);

    float c = value;

    float b=sin(x-value - t)*1.1+0.6;
    float g=sin(y+value + t)*1.1+0.6;
    float r=sin((x+y)*0.5 + t)*1.1+0.6;
    return vec4(r, g, b, 1.0);
}

void main()
{
    finalColor = plasma();
}
