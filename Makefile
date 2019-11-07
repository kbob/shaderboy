include Vars.make

        ACTIONS := build test clean install uninstall
        SUBDIRS := c/libshade c/test python
 SUBDIR_ACTIONS := $(foreach A, $(ACTIONS), $(foreach S, $(SUBDIRS), $A-$S))


$(ACTIONS): %: $(foreach S, $(SUBDIRS), %-$S)

$(SUBDIR_ACTIONS): T=$(subst -, ,$@)
$(SUBDIR_ACTIONS):
	$(MAKE) -C $(lastword $T) $(firstword $T)
