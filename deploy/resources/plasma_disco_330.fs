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

vec3 plasma(float u, float v)
{
    float t = time * 2.0;
    float x = fragTexCoord.x;
    float y = fragTexCoord.y;

    float r = sin(t + H / 20.0 + sin(t + sin(u) + cos(v)));
    float g = sin(t + H / 22.0 + sin(u + v) + cos(v - u));
    float b = sin(t + .5 + cos(u) + sin(v)) * sin(u * 20.0 * sin(u + v + t));

    r += 1.0 / dist(x, y, r, g) - cos(t/100.0);
    g += 2.0 / dist(x, y, g, r) + (sin(t / 70.0) * .5 + .5);
    b += .5 / dist(x, y, g, g) + (sin(t / 170.0) * .5 + .5);

    g /= 4.0;
    b /= 4.0;

    return vec3 (r, g, b);
}

void main()
{
    float t = time*.03;
	vec2 uv = fragTexCoord.xy * 2.0 * ((cos(t * 10.0)*.5+.5)*32.0);
    float k = cos(t * 10.0) + (cos(t)*.5+.5);
    float l = sin(t * 10.0) - (cos(t)*.5+.5);        
    uv *= mat2(k,-l,l,k); // Rotate

    finalColor = vec4(plasma(uv.x, uv.y), 1.0);

}
