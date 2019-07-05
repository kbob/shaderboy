#define R iResolution.xyz

mat2 r (float a) {
    float s = sin(a), c = cos(a);
    return mat2(c,-s,s,c);
}

vec4 ro (vec3 U) {
    vec3 v = U;
    v.yz *= r (iTime);
    v.zx *= r (iTime);
    return vec4(v,(length(v.xy)-abs(v.z)));
}

void mainCube( out vec4 Q,vec3 U )
{
    // U = 2.*(U-0.33333*R)/R.y;
    U = 2. * U;

    vec4 
        v = ro(U),
        n = ro(U+vec3(0.,1.,0.)/R),
        e = ro(U+vec3(1.,0.,0.)/R),
        s = ro(U-vec3(0.,1.,0.)/R),
        w = ro(U-vec3(1.,0.,0.)/R);
    vec3 g = normalize(vec3(e.w-w.w,n.w-s.w,.01));
    float h = dot(g,ro(vec3(1.,0.,0.)).xyz);
    Q = vec4(0.0, 0.0, 0.0, 0.0);
    Q += .2*vec4(0.5+0.5*h*h*h)*smoothstep(0.,-0.1,v.w);
    Q.rb += smoothstep(.1,.05,abs(v.w));
    Q += smoothstep(.05,0.025,abs(v.x));
    Q += smoothstep(.05,0.025,abs(v.y));
    Q += smoothstep(.05,0.025,abs(v.z));
    Q.rg += smoothstep(.25,.2,length(v.xy-v.z*vec2(cos(3.*iTime),sin(3.*iTime))));
    Q.gb += smoothstep(.25,.2,length(v.xy-v.z*vec2(-cos(3.*iTime),-sin(3.*iTime))));
    Q *= exp(-length(U));
}
