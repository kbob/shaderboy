# Interface

    Module
      init
      finit
    Executor
      start
      stop
      fps
      use_program
    Program
      create
      destroy
      is_ok
      info_log
      attach_shader(type, source)
      attach_uniform(name, type, value)
        types: 2Dx4, 3Dx4
      attach_program(name, program)

# Structure

    libshade.c
      shader.c
      program.c
      exec.c -- ???

# Threads

    python thread
    GL thread
    mpsse thread
    SPI thread

    each thread pair requires two buffers, a cond for the buffers, and
    a cond for the result.
    
    
    
    void producer()
    {
        while (1) {
            p = buffers[i];
            fill(p);
            lock();
            while (rdy == i)) {
               cond_wait(nonfull))
            i = (i + 1) % 2;
        }

    }
    
    void consumer()
    {
    }
    
# GL Textures

    glGenTextures => texture ID
    glActiveTexture - selects one of the texture units
    glBindTexture(GL_TEXTURE_2D, t_ID)  - bind to "target" -- 1 per type?
    glTexImage2D(...) - attach image data to texture.
    glGenerateMipmap() - applies to most recent texture?
    
