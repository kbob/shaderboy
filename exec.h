#ifndef EXEC_included
#define EXEC_included

#include "bcm.h"
#include "egl.h"
#include "leds.h"
#include "prog.h"

typedef struct exec exec;

extern exec *create_exec(bcm_context *, LEDs_context *);
extern void destroy_exec(exec *);

extern void exec_start(exec *);
extern void exec_stop(exec *);

extern void exec_use_prog(exec *, const prog *);

#endif /* !EXEC_included */
