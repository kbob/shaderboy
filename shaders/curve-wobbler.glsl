#!/usr/bin/env shaderbox

#define NCURVE   6    // number of curves
#define NHARM    3    // number of harmonics
#define ROLLOFF -0.8  // rolloff power - more negative is more rolloff
#define SPEED    0.1  // overall animation speed
#define EXCLUDE  0.1  // don't use a bit at the very center.
#define THICK    0.03

const float TAU   = 6.283185307179586;
const vec3  BLACK = vec3(0);
const vec3  GRAY  = vec3(0.5);
const vec3  RED   = vec3(1., 0., 0.);
const vec3  BLUE  = vec3(0., 0., 1.);

struct params {
    float phase[NHARM];
    float amp[NHARM];
    float exp;
    float fade;
};

float rand(float x) {
    return fract(100000.0 * sin(x));
}

float vary(float t, float min, float max, float seed) {
	float freq = SPEED * rand(seed);
    float phase = rand(seed + 1.2);
    float v = sin(TAU * freq * (t + phase));
    return min + (max - min) * (v + 1.) * 0.5;
}

void init(float t, int n, out params p) {
    float x = float(n + 1);
    for (int i = 0; i < NHARM; i++) {
        float h = float(i + 1);
        p.phase[i] = vary(t, 0., 1., x * h + 123.);
        p.amp[i] = vary(t, 0., pow(h, ROLLOFF), x * 234. + h * 345.);
    }
    p.exp = vary(t, 0.1, 5.0, rand(x + 456.));
    p.fade = float(n) / float(NCURVE - 1);
}

float f(float x, const params p) {

    // sum harmonics.
    float y = 0.;
    float a = 0.;
    for (int i = 0; i < NHARM; i++) {
        float h = float(i + 1);
        y += p.amp[i] * sin(h * TAU * (x + p.phase[i]));
        a += p.amp[i];
    }

    // scale to [0..1].
    y = 0.5 / a * (y + a);

    // distort with exponent.
    y = pow(y, p.exp);
    return y;
}

vec3 fg_color(params p) {
    return mix(RED, BLUE, p.fade);
}

void facemap(in vec2 fragCoord, out vec2 uv) {
    uv = fragCoord/iResolution.xy;
    float x6 = 6. * uv.x;
    float face = floor(x6);
    vec2 fxy = vec2(1. + face - x6, 1. - uv.y);
    float side_face = 5. - face;
    
    // turn face 1 sideways.
    if (face == 1.0) {
        fxy = vec2(1. - fxy.y, fxy.x);
        side_face = 3.;
    }
    
    // make map for faces 0, 1, 2, .
    uv = vec2((side_face + fxy.x) / 4., abs(2. * fxy.y - 1.));
        
    // create warp for faces 3 and 5.
    // a = angle widdershins from 7:30 o'clock.
    vec2 uvc = 2. * fxy - 1.;
    float a = mod(atan(uvc.y, uvc.x) / TAU + 0.375, 1.);
    if (face == 2.0) {
        // angle deasil from 4:30 o'clock
        a = mod(.25 - a, 1.);
    }
    
    float r0 = dot(uvc, uvc);
    float r1 = max(abs(uvc.x), abs(uvc.y));
    float r = mix(r0, r1, r1);
    r = (1. + EXCLUDE) * r - EXCLUDE;
    if (face == 0. || face == 2.) {
        uv = vec2(a, r);
    }
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv;
    facemap(fragCoord, uv);

    float x = uv.x;
    float y = uv.y;
    vec3 col = BLACK;
    for (int i = 0; i < NCURVE; i++) {
        params p;
        init(iTime, i, p);
        float fx = f(x, p);

        float brightness = smoothstep(THICK, 0.5 * THICK, abs(fx - uv.y));
        col += mix(BLACK, fg_color(p), brightness);
    }

    // col.r = uv.x;
    // col.g = uv.y;
    
    // Output to screen
    fragColor = vec4(col,1.0);
}
