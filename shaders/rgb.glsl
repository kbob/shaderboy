#!/usr/bin/env shaderbox

void mainCube(out vec4 fragColor, in vec3 fragCoord) {
    fragColor.rgb = fragCoord.xyz + .5;
}
