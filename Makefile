# PREFIX  controls where programs and libraries get installed
# Example usage:
#   make PREFIX=/usr all
@usage: make all
PWD     := $(shell pwd)
HOME    = $(PWD)/..
ODIR    = $(HOME)/eb_fg
IDIR    = $(HOME)/eb_fg
WBDIR   = $(HOME)/wb_api
DEVDIR  = $(HOME)/wb_devices
PREFIX  ?= /usr/local
SVNDIR  = $(HOME)/../production/utility/i386
INCLUDE = -I$(IDIR) -I$(WBDIR) -I$(DEVDIR)
LIBS	= -letherbone
FLAG    = DONTSIMULATE

all: 	eb_native_demo eb_wbapi_demo eb_fg_demo

eb_native_demo: $(IDIR)/eb_native_demo.c
	@echo Making eb_native_demo
	gcc -g -Wall -D$(FLAG) $(INCLUDE) -Wl,-rpath,$(PREFIX)/lib -o $(ODIR)/eb_native_demo $(IDIR)/eb_native_demo.c $(LIBS)

eb_fg_demo: $(IDIR)/eb_fg_demo.c
	@echo Making eb_fg_demo
	gcc -g -Wall -D$(FLAG) $(INCLUDE) -Wl,-rpath,$(PREFIX)/lib -o $(ODIR)/eb_fg_demo $(IDIR)/eb_fg_demo.c $(LIBS)

eb_wbapi_demo: $(IDIR)/eb_wbapi_demo.c
	@echo Making eb_wbapi_demo
	gcc -g -Wall -D$(FLAG) $(INCLUDE) -Wl,-rpath,$(PREFIX)/lib -o $(ODIR)/eb_wbapi_demo $(IDIR)/eb_wbapi_demo.c $(WBDIR)/wb_api.c $(LIBS)

install2svn:
	@echo for installing to CSCO subversion please do
	@echo 1. make PREFIX=$(SVNDIR) install
	@echo 2. dont forget to svn commit on $(PREFIX)

install:
	@echo copying stuff to $(PREFIX)
	cp $(DEVDIR)/*.h $(PREFIX)/include
	cp $(ODIR)/eb_native_demo $(PREFIX)/bin
	cp $(ODIR)/eb_wbapi_demo $(PREFIX)/bin

clean:
	@echo cleaning stuff
	rm -f eb_wbapi_demo
	rm -f eb_native_demo
	rm -f eb_fg_demo


