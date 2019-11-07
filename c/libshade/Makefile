include ../../Vars.make
include ../Rules.make

        CPPFLAGS += -I/opt/vc/include
         LDFLAGS += -L/opt/vc/lib -fvisibility=hidden -Wl,-rpath=`pwd`
          LDLIBS += -lbcm_host -lbrcmEGL -lbrcmGLESv2 -lftdi -lm -lpthread

 libshade_CFILES := shade.c bcm.c egl.c exec.c leds.c mpsse.c prog.c    \
                    queue.c render.c

 libshade_OFILES := $(libshade_CFILES:.c=.o)

         TARGETS := libshade.a libshade.so

build:	$(TARGETS)

libshade.a: $(libshade_OFILES)
	$(AR) cr $@ $^

libshade.so: $(libshade_OFILES)
	$(LINK.o) -shared -o $@ $^ $(LDLIBS)

test:	build

clean:
	rm -f *.o $(TARGETS)

install: build
	$(INSTALL) libshade.a $(PREFIX)/lib
	$(INSTALL) libshade.so $(PREFIX)/lib
	ldconfig

uninstall:
	rm -f $(PREFIX)/lib/libshade.a
	rm -f $(PREFIX)/lib//libshade.so
