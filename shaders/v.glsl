#!/usr/bin/env shaderbox

#define TAU 6.283185307179586

#define A vec3(0.5, 0.5, 0.5)
#define B vec3(0.5, 0.5, 0.5)
#define C vec3(1.0, 0.7, 0.4)
#define D vec3(0.0, 0.15, 0.20)

vec3 pal(in float t, in vec3 a, in vec3 b, in vec3 c, in vec3 d)
{
    return a + b * cos(TAU * (c * t + d));
}

vec3 hash32(vec2 p)
{
    // Dave Hoskins CC BY-SA 4.0
    vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yxz+19.19);
    return fract((p3.xxy+p3.yzz)*p3.zyx);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec2 divisions = vec2(24, 4);
    vec2 st = divisions * fragCoord / iResolution.xy;

    vec2 i_st = floor(st);
    vec2 f_st = mod(st, 1.0);

    float min_dist = 1000.0;
    float hue;

    for (int i = -1; i <= +1; i++) {
        for (int j = -1; j <= +1; j++) {
            vec2 neighbor = vec2(i, j);
            vec3 h = hash32(i_st + neighbor);
            vec2 center = h.xy;;
            center = 0.5 + 0.4 * sin(iTime + 6.2831 * center);
            float dist = distance(f_st, neighbor + center);
            if (min_dist > dist) {
                min_dist = dist;
                hue = h.z;
            }
        }
    }
    vec3 color = pal(fract(hue + 0.2 * iTime), A, B, C, D);
    // vec3 color = vec3(hue);
    color -= abs(sin(15. * min_dist)) * 0.2;
    color += 1. - step(0.02, min_dist);
    // color.r += step(.99, f_st.x) + step(.99, f_st.y);

    fragColor.rgb = color;
}
