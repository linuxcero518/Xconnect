CXX         := g++
CXXFLAGS    := $(shell fltk-config --cxxflags) -std=c++20 -O2 -Wall -Wextra -Wpedantic -Iinclude
LDFLAGS     := $(shell fltk-config --ldflags)
GLIB_CFLAGS := $(shell pkg-config --cflags gio-2.0)
GLIB_LIBS   := $(shell pkg-config --libs gio-2.0)

TARGET := xconnect
HELPER := xconnect-helper

SRC := $(shell find src -name '*.cpp' ! -path 'src/helper/*')
OBJ := $(SRC:.cpp=.o)
DEP := $(OBJ:.o=.d)

HELPER_SRC := src/helper/helper.cpp
HELPER_OBJ := $(HELPER_SRC:.cpp=.o)
HELPER_DEP := $(HELPER_OBJ:.o=.d)

PREFIX     ?= /usr
BINDIR     ?= $(PREFIX)/bin
LIBEXECDIR ?= $(PREFIX)/libexec
APPDIR     ?= $(PREFIX)/share/applications
POLICYDIR  ?= $(PREFIX)/share/polkit-1/actions
DBUSDIR    ?= $(PREFIX)/share/dbus-1/system.d
SERVICEDIR ?= $(PREFIX)/share/dbus-1/system-services

.PHONY: all clean run install remove

all: $(TARGET) $(HELPER)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS) $(GLIB_LIBS)

$(HELPER): $(HELPER_OBJ)
	$(CXX) -o $@ $^ $(GLIB_LIBS)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $(GLIB_CFLAGS) -MMD -MP -c -o $@ $<

run: $(TARGET)
	./$(TARGET)

install: all
	install -Dm755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)
	install -Dm755 $(HELPER) $(DESTDIR)$(LIBEXECDIR)/$(HELPER)
	install -Dm644 data/xconnect.desktop $(DESTDIR)$(APPDIR)/xconnect.desktop
	install -Dm644 data/org.xconnect.Helper.policy $(DESTDIR)$(POLICYDIR)/org.xconnect.Helper.policy
	install -Dm644 data/org.xconnect.Helper.conf $(DESTDIR)$(DBUSDIR)/org.xconnect.Helper.conf
	install -Dm644 data/org.xconnect.Helper.service $(DESTDIR)$(SERVICEDIR)/org.xconnect.Helper.service

remove:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -f $(DESTDIR)$(LIBEXECDIR)/$(HELPER)
	rm -f $(DESTDIR)$(APPDIR)/xconnect.desktop
	rm -f $(DESTDIR)$(POLICYDIR)/org.xconnect.Helper.policy
	rm -f $(DESTDIR)$(DBUSDIR)/org.xconnect.Helper.conf
	rm -f $(DESTDIR)$(SERVICEDIR)/org.xconnect.Helper.service

clean:
	rm -f $(TARGET) $(HELPER) $(OBJ) $(DEP) $(HELPER_OBJ) $(HELPER_DEP)

-include $(DEP) $(HELPER_DEP)