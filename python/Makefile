include ../Vars.make

 TARGETS := shaderbox shade/__init__.py

build:	$(TARGETS)

test:
	./pytest

clean:
	rm -f *.pyc
	rm -rf __pycache__ */__pycache__ build dist *.egg-info

install: build
        # Pip refuses to install if the package is in the current directory.
	cd /tmp && \
           $(PYTHON) -m pip install --prefix=$(PREFIX) --upgrade $(CURDIR)
	$(INSTALL) shaderbox $(PREFIX)/bin

uninstall:
        # Pip refuses to deinstall if the package is in the current directory.
	cd /tmp && $(PYTHON) -m pip uninstall $(CURDIR)
	rm -f $(PREFIX)/bin/shaderbox
