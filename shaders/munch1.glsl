#!/usr/bin/env shaderbox

mat3 xform(float time)
{
    // return mat3(1.,0.,0.,0.,1.,0.,0.,0.,1);
    float rx = time * 0.13;
    float ry = time * 0.8;
    float rz = time * 0.3;
    mat3 mx = mat3(
         1.0,  0.0,     0.0,
         0.0,  cos(rx), sin(rx),
         0.0, -sin(rx), cos(rx)
    );
    mat3 my = mat3(
         cos(ry), 0.0, sin(ry),
         0.0,     1.0, 0.0,
        -sin(ry), 0.0, cos(ry)
    );
    mat3 mz = mat3(
         cos(rz), sin(rz), 0.0,
        -sin(rz), cos(rz), 0.0,
         0.0,     0.0,     1.0
    );
    return mx * my * mz;
}

void decompose(in float c, out float bits[6])
{
    float c6 = c;
    float c5 = floor(c6 * 2.);
    float c4 = floor(c6 * 4.);
    float c3 = floor(c6 * 8.);
    float c2 = floor(c6 * 16.);
    float c1 = floor(c6 * 32.);
    float c0 = floor(c6 * 64.);
    c6 = floor(c6);

    bits[5] = c5 - 2. * c6;
    bits[4] = c4 - 2. * c5;
    bits[3] = c3 - 2. * c4;
    bits[2] = c2 - 2. * c3;
    bits[1] = c1 - 2. * c2;
    bits[0] = c0 - 2. * c1;
}

float compose(in float bits[6])
{
    return (bits[0] * (1. / 64.) +
            bits[1] * (1. / 32.) +
            bits[2] * (1. / 16.) +
            bits[3] * (1. /  8.) +
            bits[4] * (1. /  4.) +
            bits[5] * (1. /  2.));
}

float xor(in float a, in float b)
{
    float a_bits[6];
    float b_bits[6];
    float c_bits[6];
    decompose(a, a_bits);
    decompose(b, b_bits);
    for (int i = 0; i < 6; i++) {
        c_bits[i] = a_bits[i] == b_bits[i] ? 0. : 1.;
    }
    return compose(c_bits);
}

float cond(float c)
{
    return floor(mod(c * 64., 64.)) / 64.;
}

void mainCube(out vec4 fragColor, in vec3 fragCoord)
{
    // Normalized pixel coordinates (from 0 to 1)

    vec3 uvw = fragCoord * xform(iTime);
    float xx = xor(cond(uvw.x), xor(cond(uvw.y), cond(uvw.z)));
    float zz = mod(float(iFrame / 2) / 64., 1.);
    
    float adxz = abs(xx - zz);
    bool mun = abs(xx - zz) <= 1./64.;
    float zs = 2. * abs(zz - 0.5);

    // Time varying pixel color
    float r = abs(xx - zs);
    float g = 0.5 - r;
    g *= smoothstep(0.0, 0.05, adxz);
    float b = mun ? 1. : 0.;

    vec3 col = vec3(0);
    // col.r = b * 0.4;
    col.g = g;
    col.b = b;
    col = max(vec3(0.01), col); // replace black with dark grey.

    // Output to screen
    fragColor = vec4(col, 1.0);
}
