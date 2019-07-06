#!/usr/bin/env shaderbox

// Based on https://www.shadertoy.com/view/Xd3SW8 by Hanley

#define TAU 6.283185307179586
#define PI 3.141592653589793
#define QTAU 1.5707963267948966

float polygonDistanceField(in vec2 pixelPos, in int N) {
    // N = number of corners
    float a = atan(pixelPos.y, pixelPos.x) + QTAU; // angle
    float r = TAU / float(N);
    // shaping function that modulates the distances
    float distanceField = cos(floor(0.5 + a / r) * r - a) * length(pixelPos);
    return distanceField;
}

float minAngularDifference(in float angleA, in float angleB) {
    // Ensure input angles are -Ï€ to Ï€
    angleA = mod(angleA, TAU);
    if (angleA>PI) angleA -= TAU;
    if (angleA<PI) angleA += TAU;
    angleB = mod(angleB, TAU);
    if (angleB>PI) angleB -= TAU;
    if (angleB<PI) angleB += TAU;

    // Calculate angular difference
    float angularDiff = abs(angleA - angleB);
    angularDiff = min(angularDiff, TAU - angularDiff);
    return angularDiff;
}

float map(in float value,
          in float istart, in float istop,
          in float ostart, in float ostop) {
    return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
}

float mapAndCap(in float value,
                in float istart, in float istop,
                in float ostart, in float ostop) {
    float v = map(value, istart, istop, ostart, ostop);
    v = max(min(ostart, ostop), v);
    v = min(max(ostart, ostop), v);
    return v;
}

// rotate matrix
mat2 rotate2d(float angle) {
    return mat2(cos(angle), -sin(angle),
                sin(angle),  cos(angle) );
}

mat3 getPT(in vec3 p) {
    vec3 a = abs(p);
    float c = max(max(a.x, a.y), a.z);    

    vec3 s;
    if (c == a.x)
        s = vec3(1, 0, 0);
    else if (c == a.y)
        s = vec3(0, 1, 0);
    else
        s = vec3(0, 0, 1);
    s *= sign(dot(p, s));

    vec3 q = s.yzx;
    return mat3(cross(q, s), q, s);
}

float axis_to_face(vec3 axis) {
    float idx = dot(abs(axis), vec3(0., 2., 4.));
    if (dot(axis, vec3(1.)) < 0.)
        idx += 1.;
    return idx;
}

void mainCube(out vec4 fragColor, in vec3 fragCoord)
{
    float t = iTime;

    mat3 PTn = getPT(fragCoord);
    vec3 xyz = fragCoord + 0.5;  // range [0..1]
    vec2 st = (xyz * PTn).xy;

    // manip st grid - into 3x3 tiles
    float divisions = 3.;
    vec3 mxyz = xyz * divisions;
    vec2 mst = st * divisions;

    // tile mst and mxyz
    vec2 imst = floor(mst);
    vec2 fmst = mod(mst, 1.);
    vec3 imxyz = floor(mxyz);
    vec3 fmxyz = mod(mxyz, 1.);
    
    // draw square tile
    
    vec3 imxyz3 = imxyz * .3;
    float tt = t - 0.15 * (cos(imxyz3.x) - sin(imxyz3.y) - cos(imxyz3.z));
    tt = max(tt, 0.);

    float squareProgress = mod(tt * .3, 1.);
    float squareEntryProgress = mapAndCap(squareProgress, 0., 0.6, 0., 1.);
    float squareExitProgress = mapAndCap(squareProgress, 0.9, .999, 0., 1.);
    squareExitProgress = pow(squareExitProgress, 3.);

    float borderProgress = mapAndCap(squareEntryProgress,0., 0.55, 0., 1.);
    borderProgress = pow(borderProgress, 1.5);
    float fillProgress = mapAndCap(squareEntryProgress, 0.4, 0.9, 0., 1.);
    fillProgress = pow(fillProgress, 4.);

    // MATRIX MANIP
    fmst = fmst * 2. - 1.; // centre origin point
    // rotate
    mat2 rot = rotate2d(QTAU * (imst.x + imst.y) + t * 0.4);
    fmst = rot * fmst;

    float d = polygonDistanceField(fmst, 4);
    float r = map(squareExitProgress, 0., 1., sqrt(0.5), 0.);
    float innerCut = map(fillProgress, 0., 1., 0.8, 0.0001);
    float buf = 1.01;
    float shape = smoothstep(r * buf, r, d) - smoothstep(r * innerCut, r * innerCut / buf, d);
    // add smoother shape glow
    // buf = 1.5;
    // float shape2 = smoothstep(r * buf, r, d) - smoothstep(r * innerCut, r * innerCut / buf, d);
    // shape += shape2 * .5;


    // angular mask on square tile
    float sta = atan(fmst.y, fmst.x); // st-angle - technically its fmsta here
    float targetAngle = map(borderProgress, 0., 1., 0., PI) + PI * .251;
    float adiff = minAngularDifference(sta, targetAngle);
    float arange = map(borderProgress, 0., 1., 0., PI);
    float amask = 1. - smoothstep(arange, arange, adiff);
    shape *= amask;


    // color
    // color = vec3(shape) * vec3(0.8, 0.6, 0.8) * 2.;
    
    float c = dot(xyz, xyz) * (1. / 3.);
    vec3 color = vec3(1. - c, c, c) + 0.2;
    color *= shape;
    
    fragColor = vec4(color, 1.0);
}
