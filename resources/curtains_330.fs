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

	vec2 uv = fragTexCoord.xy * ((cos(t)*.5+.5+.5)*127.0);
    float k = cos(t * .5) * (cos(t * .1)*.5+.5) * 1.5;
    float l = sin(t * .5) * (cos(t * .1)*.5+.5) * 1.7;
    uv *= mat2(k,l,-l,k); // Rotate
    u = uv.x;
    v = uv.y;

    float r = sin(sin(t) + .9 + cos(t + sin(u) + cos(v)));
    float g = sin(t + .7 + cos(v + u));
    float b = sin(t + .5 + cos(u) + sin(v)) * sin(u * 20.0 * sin(u + v + t)) / W;

    r += dist(x, y, r, g);
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
    finalColor = vec4(curtains(), 1.0);
}
