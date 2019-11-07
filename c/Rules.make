 CPPFLAGS :=
   CFLAGS := -g -Wall -Werror -fpic -Wmissing-prototypes
  LDFLAGS := -g -L/opt/vc/lib
   LDLIBS := -lbcm_host -lbrcmEGL -lbrcmGLESv2 -lftdi -lm -lpthread
