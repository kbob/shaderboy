     CPPFLAGS := -I/opt/vc/include
       CFLAGS := -g -Wall -Werror -fpic -Wmissing-prototypes
      LDFLAGS := -g -L/opt/vc/lib -fvisibility=hidden -Wl,-rpath=`pwd`
       LDLIBS := -lbcm_host -lbrcmEGL -lbrcmGLESv2 -lftdi -lm -lpthread

 ptest_CFILES := ptest.c queue.c
 ptest_OFILES := $(ptest_CFILES:.c=.o)

 shade_CFILES := shade.c bcm.c egl.c exec.c leds.c mpsse.c prog.c	\
		 queue.c render.c

 shade_OFILES := $(shade_CFILES:.c=.o)

      TARGETS := libshade.a libshade.so ptest ltest-static ltest-dynamic

all:	$(TARGETS)

ltest-static: ltest.o libshade.a
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

ltest-dynamic: ltest.o libshade.so
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

ptest: $(ptest_OFILES)

libshade.a: $(shade_OFILES)
	$(AR) cr $@ $^

libshade.so: $(shade_OFILES)
	$(LINK.o) -shared -o $@ $^ $(LDLIBS)

clean:
	rm -f *.o $(TARGETS)
