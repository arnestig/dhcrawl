PROGNAME=dhcrawl

CXX = g++
INSTALL = install -o root -g root -m 755
INSTALL_DIR = install -p -d -o root -g root -m 755
INSTALL_DATA = install -p -o root -g root -m 644
CFLAGS += 
CPPFLAGS +=
CXXFLAGS += -g -Wall
LDFLAGS += -pthread -lrt

ifneq (,$(filter noopt,$(DEB_BUILD_OPTIONS)))
	CXXFLAGS += -O0
else
	CXXFLAGS += -O2
endif
ifeq (,$(filter nostrip,$(DEB_BUILD_OPTIONS)))
	INSTALL += -s
endif
ifneq (,$(filter parallel=%,$(DEB_BUILD_OPTIONS)))
	NUMJOBS = $(patsubst parallel=%,%,$(filter parallel=%,$(DEB_BUILD_OPTIONS)))
	MAKEFLAGS += -j$(NUMJOBS)
endif

OBJFILES := $(patsubst src/%.cpp,obj/%.o,$(wildcard src/*.cpp))

all: $(PROGNAME)

$(PROGNAME): $(OBJFILES) 
	$(CXX) -o $(PROGNAME) $(INCLUDE_DIR) $(OBJFILES) $(LDFLAGS)

obj/%.o: src/%.cpp 
	@mkdir -p obj
	$(CXX) -c $< -o $@ $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS)

clean:
	rm -f $(OBJFILES) $(PROGNAME)

rebuild: clean all

install:
	$(INSTALL_DIR) $(DESTDIR)/usr/sbin
	$(INSTALL) $(PROGNAME) $(DESTDIR)/usr/sbin

uninstall:
	rm -f $(DESTDIR)/usr/sbin/$(PROGNAME)

.PHONY: install uninstall

