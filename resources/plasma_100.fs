#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Custom inputs
uniform float W;
uniform float H;
uniform float time;

float dist(float x1, float y1, float x2, float y2)
{
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

vec4 plasma()
{
    float value = 0.0;
    float t = time / 60.0;

    float x = fragTexCoord.x / W;
    float y = fragTexCoord.y / H;

    float d1 = dist(x, y, .3, .8) * W * 12.0;
    float d2 = dist(x, y, .8, .1) * W * 11.0;
    float d3 = dist(x, y, .1, .8) * W * 10.0;
    float d4 = dist(x, y, t / 100.0, t / 100.0)* W * 11.5;
    value = 0.5 + 0.5 * sin(d1) + sin(d2) + sin(d3) + sin(d4);

    float c = value;

    float b=sin(x-value - t)*1.1+0.6;
    float g=sin(y+value + t)*1.1+0.6;
    float r=sin((x+y)*0.5 + t)*1.1+0.6;
    return vec4(r, g, b, 1.0);
}

void main()
{
    //finalColor = plasma();
    gl_FragColor = plasma();
}
