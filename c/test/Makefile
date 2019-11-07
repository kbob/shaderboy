include ../../Vars.make
include ../Rules.make

 LIBSHADE_DIR := ../libshade
   LIBSHADE_A := $(LIBSHADE_DIR)/libshade.a
  LIBSHADE_SO := $(LIBSHADE_DIR)/libshade.so

     CPPFLAGS += -I$(LIBSHADE_DIR)

 ptest_CFILES := ptest.c $(LIBSHADE_DIR)/queue.c
 ptest_OFILES := $(ptest_CFILES:.c=.o)
 ptest_LDLIBS := -lpthread

      TARGETS := ptest ltest-static ltest-dynamic

build:	$(TARGETS)

ptest:	LDLIBS := $(ptest_LDLIBS)
ptest:	$(ptest_OFILES)

ltest-static: LDLIBS += $(LIBSHADE_A)
ltest-static: ltest.o $(LIBSHADE_A)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

ltest-dynamic: LDLIBS += $(LIBSHADE_SO)
ltest-dynamic: ltest.o $(LIBSHADE_SO)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

test:	build
	./ptest
	./ltest-static
	./ltest-dynamic

clean:
	rm -f *.o $(TARGETS)

install: build

uninstall:
