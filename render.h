#ifndef RENDER_included
#define RENDER_included

#include "bcm.h"
#include "prog.h"

typedef struct render_state render_state;

extern render_state *render_init(const bcm_context);
extern void          render_deinit(render_state *);
extern void          render_frame(render_state *, const prog *);

#endif /* !RENDER_included */
