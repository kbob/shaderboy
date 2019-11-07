# Shaderboy

Shaderboy is an application and library to render OpenGL shaders
on LED panels and cubes.  It is optimized for and coupled to
the Raspberry Pi and the iCEBreaker FPGA.

Shaderboy draws inspiration from [Shadertoy](https://shadertoy.com)
and [Shady](https://github.com/polyfloyd/shady).


# Installation

```sh
$ make && sudo make install
```

# Where's the Eye Candy?

Most of the demos are in the `shaders` directory.

```sh
cd shaders
for demo in *.glsl
do
    shaderbox --duration=10 $demo
done
```

Some additional demos are in the `scripts` directory.


# Architecture

Shaderboy has several public APIs: Linux CLI, Python, and C.

## Command line 

At the highest level, the `shaderbox` command renders a fragment
shader written in the OpenGL Shading Langues (GLSL) on the LEDs.

```sh
$ echo 'void main() { gl_FragColor = vec4(1); }' | shaderbox
```

or

```sh
$ shaderbox myfile.glsl
```

## Python API

You can also use shaderboy from Python programs using the `shade`
module.  See the `scripts` subdirectory for examples.


## C API

You can use shaderboy from C programs using the `libshade` library.
See `c/test/ltest.c` for an example.


# GLSL extensions

The Raspberry Pi uses GLSL 1.0.  Shaderboy adds some extensions
via a preprocessor.

## Predefined Variables

To be documented.  Similar to Shadertoy's.
`iResolution`, `iTime`, `iFrame` are implemented.

## Alternate Entrypoints

To be documented.  Similar to Shady's.
`main`, `mainImage`, `mainCube` are implemented.


## Pragmas

To be documented.  Similar to Shady's.
```c
    #pragma use [filename]
    #pragma map [variable]=image:[filename]
    #pragma map [variable]=builtin:[builtin]
    #pragma map [variable]=perip_map4:[device]
```


# Optimization

Shaderboy creates three threads and does all rendering in those
threads.  Rendering is asynchronous with the calling thread.
On a four core Raspberry Pi, one core is never blocked by
rendering.

 * The **render** thread invokes the GPU to render pixels on screen.
   This uses OpenGL.
 
 * The **cmd** thread repackages pixels as commands for the FTDI chip
   on the iCEBreaker board.
   
 * The **output** thread sends commands to the iCEBreaker
   via the Raspberry Pi's USB interface.

The render thread may be CPU or GPU bound.  The output thread is
usually waiting on the FTDI chip.  And the cmd thread is there to
offload and decouple the other two threads so they can always run.
