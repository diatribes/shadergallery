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

vec3 plasma()
{
    float t = time;
    float x = fragTexCoord.x;
    float y = fragTexCoord.y;
    float u = x * 8.0;
    float v = y * 8.0;

	vec2 uv = fragTexCoord.xy * ((cos(t)*.5+.5+.5)*127.0);
    float k = cos(t * .9) * (cos(t * .1)*.5+.5) * .83;
    float l = sin(t * .5) * (cos(t * .7)*.5+.5) * .33;
    uv *= mat2(k,-l,l,k); // Rotate
    u = uv.x;
    v = uv.y;

    float r = sin(sin(t) + .9 + cos(t + sin(u) + cos(v)));
    float g = sin(t + .7 + cos(v + u));
    float b = sin(t + .5 + cos(u) + sin(v)) * sin(u * 20.0 * sin(u + v + t)) / (W / 2.0);

    r += 1.0 /dist(x, y, r, g);
    g += 1.0/ dist(x, y, g, r) + (sin(t / 70.0) * .5 + .5) / 10.0;
    b += dist(x, y, r, g) + (sin(t / 170.0) * .5 + .5) / 10.0;

    float value = abs(sin(r) + sin(g) + sin(b) + sin(u) + sin(v)) * 2.0;

    r /= value;
    g /= value;
    b /= value;

    return vec3 (r, g, b);
}

void main()
{
    //finalColor = plasma();
    gl_FragColor = vec4(plasma(), 1.0);
}
