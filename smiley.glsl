#!/usr/bin/env shaderbox

#pragma map face=image:smiley.png

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;
    vec2 uv6 = uv * vec2(6.0, 1.0);

    // Time varying pixel color
    vec3 col = 0.5 + 0.5 * cos(iTime + uv6.xyx + vec3(0.0, 2.0, 4.0));
    vec3 c2 = texture2D(face, uv6 + iTime * 0.1).rgb;
    c2 = c2 * c2;
    col *= c2;

    // Output to screen
    fragColor = vec4(col, 1.0);
}
