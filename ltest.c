// Test library.

#include <stdio.h>
#include <unistd.h>

#include "shade.h"

static const int LEDS_WIDTH  = 6 * 64;
static const int LEDS_HEIGHT = 64;

static const char vertex_shader_source[] =
    "attribute vec3 vert;\n"
    "\n"
    "void main(void) {\n"
    "    gl_Position = vec4(vert, 1.0);\n"
    "}\n"
    ;

static const char frag_shader_source[] =
    "uniform vec3 iResolution;\n"
    "uniform float iTime;\n"
    "uniform float iTimeDelta;\n"
    "uniform float iFrame;\n"
    "uniform float iChannelTime[4];\n"
    "uniform vec4 iMouse;\n"
    "uniform vec4 iDate;\n"
    "uniform float iSampleRate;\n"
    "uniform vec3 iChannelResolution[4];\n"
    "const float PI = 3.141592;\n"
    "const float EPSILON = 0.0001;\n"
    "\n"
    "// Maps 2D output image space to 3D cube space.\n"
    "//\n"
    "// The returned coordinates are in the range of (-.5, .5).\n"
    "vec3 cube_map_to_3d(vec2 pos) {\n"
    "    vec3 p = vec3(0.0);\n"
    "    if (pos.x < 128.0) {\n"
    "        // top\n"
    "        p = vec3(1.0 - pos.y / 128.0,\n"
    "                 1.0,\n"
    "                 pos.x / 128.0);\n"
    "    } else if (pos.x < 256.0) {\n"
    "        // back\n"
    "        p = vec3(1.0 - pos.y / 128.0,\n"
    "                 1.0 - (pos.x - 128.0) / 128.0,\n"
    "                 1.0);\n"
    "    } else if (pos.x < 384.0) {\n"
    "        // bottom\n"
    "        p = vec3(1.0 - pos.y / 128.0,\n"
    "                 0.0,\n"
    "                 1.0 - (pos.x - 256.0) / 128.0);\n"
    "    } else if (pos.x < 512.0) {\n"
    "        // right\n"
    "        p = vec3(1.0,\n"
    "                 1.0 - pos.y / 128.0,\n"
    "                 1.0 - (pos.x - 384.0) / 128.0);\n"
    "    } else if (pos.x < 640.0) {\n"
    "        // front\n"
    "        p = vec3(1.0 - (pos.x - 512.0) / 128.0,\n"
    "                 1.0 - pos.y / 128.0,\n"
    "                 0.0);\n"
    "    } else if (pos.x < 768.0) {\n"
    "        // left\n"
    "        p = vec3(0,\n"
    "                 1.0 - pos.y / 128.0,\n"
    "                 (pos.x - 640.0) / 128.0);\n"
    "    }\n"
    "    return p - 0.5;\n"
    "}\n"
    "\n"
    " void mainCube(out vec4 fragColor, in vec3 fragCoord) {\n"
    "     fragColor.rgb = fragCoord.xyz + .5;\n"
    "}\n"
    "\n"    
    "#ifndef _EMULATOR\n"
    "void mainImage(out vec4 fragColor, in vec2 fragCoord) {\n"
    "     mainCube(fragColor, cube_map_to_3d(fragCoord));\n"
    "}\n"
    "#endif\n"
    "\n"    
    "void main() {\n"
    "    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
    "    mainImage(gl_FragColor, gl_FragCoord.xy);\n"
    "    gl_FragColor.a = 1.0;\n"
    "}\n"
    ;

int main()
{
    shd_init(LEDS_WIDTH, LEDS_HEIGHT);
    shd_prog *prog = shd_create_prog();
    shd_prog_attach_shader(prog, SHD_SHADER_VERTEX, vertex_shader_source);
    shd_prog_attach_shader(prog, SHD_SHADER_FRAGMENT, frag_shader_source);
    char *info_log = NULL;
    if (!shd_prog_is_okay(prog, &info_log)) {
        fprintf(stderr, "info: %s\n", info_log);
        return 1;
    }
    shd_use_prog(prog);

    shd_start();
    sleep(1);
    shd_stop();
    shd_destroy_prog(prog);
    shd_deinit();
    return 0;
}
