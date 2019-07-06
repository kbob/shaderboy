# pragma map alphabet = image:alphabet6.png

/* RGB from hue. */
vec3 hue(float h) {
    vec3 c = mod(h*6.0 + vec3(2, 0, 4), 6.0);
    return h >= 1.0 ? vec3(h-1.0) : clamp(min(c, -c+4.0), 0.0, 1.0);
}

/* Return a permutation matrix whose first two columns are u and v basis 
   vectors for a cube face, and whose third column indicates which axis 
   (x,y,z) is maximal. */
mat3 getPT(in vec3 p) {
    vec3 a = abs(p);
    float c = max(max(a.x, a.y), a.z);    

    vec3 s = c == a.x ? vec3(1.,0,0) : c == a.y ? vec3(0,1.,0) : vec3(0,0,1.);

    s *= sign(dot(p, s));
    vec3 q = s.yzx;
    return mat3(cross(q,s), q, s);
}


vec3 maj_axis(vec3 p) {
    vec3 a = abs(p);
    float c = max(a.x, max(a.y, a.z));
    vec3 s;
    if (c == a.x)
        s = vec3(1, 0, 0);
    else if (c == a.y)
        s = vec3(0, 1, 0);
    else
        s = vec3(0, 0, 1);
    s *= sign(dot(p, s));  // -1 for negative axis
    return s;
}

float axis_to_face(vec3 axis) {
    float idx = dot(abs(axis), vec3(0., 2., 4.));
    if (dot(axis, vec3(1.)) < 0.)
        idx += 1.;
    return idx;
}

void mainCube(out vec4 fragColor, in vec3 fragCoord)
{
    mat3 PTn = getPT(fragCoord);
    vec3 st = fragCoord * PTn;
    float f = axis_to_face(PTn[2]);

    vec2 st26 = vec2((st.x + 0.5 + f) / 6., st.y + 0.5);
    vec4 b = texture2D(alphabet, st26);
    // vec3 b = abs(PTn[2]) * vec3((st.x + st.y) / 2.);
    vec3 face_col = abs(PTn[2]);

    vec3 col = vec3(1.);
    col -= (1. - face_col) * b.r;
    col -= (1. - face_col) * b.g;

    // fragColor.rgb = b.rgb * 0.3;
    fragColor.rgb = col.rgb;
}
