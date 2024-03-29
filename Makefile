# ===========================================================================
#                          CONFIGURATION OPTIONS
# ===========================================================================

# It should be mostly safe to leave these options at the default.

PREFIX ?= /home/kuzmich/src/wordgrinder/pkg/wordgrinder/usr
BINDIR ?= $(PREFIX)/bin
SHAREDIR ?= $(PREFIX)/share
DOCDIR ?= $(SHAREDIR)/doc
MANDIR ?= $(SHAREDIR)/man
DESTDIR ?=

# Where do the temporary files go?
OBJDIR = .obj

# The compiler used for the native build (curses, X11)
CC ?= cc

# Which ninja do you want to use?
ifeq ($(strip $(shell type ninja >/dev/null; echo $$?)),0)
	NINJA ?= ninja
else
	ifeq ($(strip $(shell type ninja-build >/dev/null; echo $$?)),0)
		NINJA ?= ninja-build
    else
        $(error No ninja found)
    endif
endif

# Global CFLAGS and LDFLAGS.
CFLAGS ?=
LDFLAGS ?=

# Used for the Windows build (either cross or native)
WINCC ?= i686-w64-mingw32-gcc
WINDRES ?= i686-w64-mingw32-windres
MAKENSIS ?= makensis
ifneq ($(strip $(shell type $(MAKENSIS) >/dev/null 2>&1; echo $$?)),0)
	# If makensis isn't on the path, chances are we're on Cygwin doing a
	# Windows build --- so look in the default installation directory.
	MAKENSIS := /cygdrive/c/Program\ Files\ \(x86\)/NSIS/makensis.exe
endif

# Application version and file format.
VERSION := 0.8
FILEFORMAT := 8

ifdef SOURCE_DATE_EPOCH
       DATE := $(shell LC_ALL date --utc --date="@$(SOURCE_DATE_EPOCH)" +'%-d %B %Y')
else
       DATE := $(shell date +'%-d %B %Y')
endif

# Which Lua do you want to use?
#
# Use 'builtin' if you want to use the built-in Lua 5.1. If
# you want to dynamically link to your system's Lua, or to Luajit,
# use a pkg-config name instead (e.g. lua-5.1, lua-5.2, luajit).
# WordGrinder works with 5.1, 5.2, 5.3, and LuaJit.
#
# Alternatively, use a flag specifier string like this:
# --cflags={-I/usr/include/thingylua} --libs={-L/usr/lib/thingylua -lthingylua}
LUA_PACKAGE ?= builtin

# Hack to try and detect the presence of the Xft library (it's not in
# pkg-config).
ifneq ($(wildcard /usr/include/X11/Xft/Xft.h),)
	XFT_PACKAGE ?= \
		--cflags={-I/usr/include/X11} --libs={-lX11 -lXft}
else ifneq ($(wildcard /usr/X11R6/include/X11/Xft/Xft.h),)
	XFT_PACKAGE ?= \
		--cflags={-I/usr/X11R6/include -I/usr/X11R6/include/X11} \
		--libs={-L/usr/X11R6/lib -lX11 -lXft}
else
	XFT_PACKAGE ?= none
endif

# Hack to try and detect OSX's non-pkg-config compliant ncurses.
ifneq ($(filter Darwin%,$(shell uname)),)
	CURSES_PACKAGE ?= --cflags={-I/usr/include} --libs={-L/usr/lib -lncurses}
else ifneq ($(filter OpenBSD,$(shell uname)),)
	CURSES_PACKAGE ?= --cflags={-I/usr/include} --libs={-L/usr/lib -lncurses}
else
	CURSES_PACKAGE ?= ncursesw
endif

# By default, WordGrinder uses the builtin versions of these libraries.
# However, they're overridable --- this is mainly of use if you're a
# package maintainer and want to dynamically link to your platform's
# version.
#
# Important note: the pkg-config files for Lua packages are typically
# wrong, as they'll try to link in the wrong Lua library. You'll
# probably have to use a manual flag specifier string. Also, setting
# these only makes sense with 'make all' --- don't use this with
# 'make dev' (but you probably won't be doing this anyway).

LPEG_PACKAGE ?= builtin
LUABITOP_PACKAGE ?= builtin
MINIZIP_PACKAGE ?= builtin
UTHASH_PACKAGE ?= builtin

# Do you want your binaries stripped on installation?

WANT_STRIPPED_BINARIES ?= yes

# ===========================================================================
#                       END OF CONFIGURATION OPTIONS
# ===========================================================================
#
# If you need to edit anything below here, please let me know so I can add
# a proper configuration option.

hide = @

LUA_INTERPRETER = $(OBJDIR)/lua

NINJABUILD = \
	$(hide) $(NINJA) -f $(OBJDIR)/build.ninja $(NINJAFLAGS)

# Builds and tests the Unix release versions only.
.PHONY: all
all: $(OBJDIR)/build.ninja
	$(NINJABUILD) all

# Builds, tests and installs the Unix release versions only.
.PHONY: install
install: $(OBJDIR)/build.ninja
	$(NINJABUILD) install

# Builds and installs the Unix release versions only, without testing.
.PHONY: install-notests
install-notests: $(OBJDIR)/build.ninja
	$(NINJABUILD) install-notests

# Builds and tests everything that's buildable on your machine. Don't use this
# unless you know what you're doing (it's pretty brittle given non-standard
# build flags).
.PHONY: dev
dev: $(OBJDIR)/build.ninja
	$(NINJABUILD) dev

# Builds Windows, possibly via cross-compilation (but doesn't test it because
# we can only do that on actual Windows).
.PHONY: windows
windows: $(OBJDIR)/build.ninja
	$(NINJABUILD) bin/WordGrinder-$(VERSION)-setup.exe 

# Run the Windows tests
.PHONY: wintests
wintests: $(OBJDIR)/build.ninja
	$(NINJABUILD) wintests

.DELETE_ON_ERROR:

$(OBJDIR)/build.ninja:: $(LUA_INTERPRETER) build.lua Makefile
	@mkdir -p $(dir $@)
	$(hide) $(LUA_INTERPRETER) build.lua \
		BINDIR="$(BINDIR)" \
		BUILDFILE="$@.tmp" \
		CC="$(CC)" \
		CFLAGS="$(CFLAGS)" \
		CURSES_PACKAGE="$(CURSES_PACKAGE)" \
		DATE="$(DATE)" \
		DESTDIR="$(DESTDIR)" \
		DOCDIR="$(DOCDIR)" \
		FILEFORMAT="$(FILEFORMAT)" \
		LDFLAGS="$(LDFLAGS)" \
		LUABITOP_PACKAGE="$(LUABITOP_PACKAGE)" \
		LPEG_PACKAGE="$(LPEG_PACKAGE)" \
		LUA_INTERPRETER="$(LUA_INTERPRETER)" \
		LUA_PACKAGE="$(LUA_PACKAGE)" \
		MAKENSIS="$(MAKENSIS)" \
		MANDIR="$(MANDIR)" \
		MINIZIP_PACKAGE="$(MINIZIP_PACKAGE)" \
		OBJDIR="$(OBJDIR)" \
		SHAREDIR="$(SHAREDIR)" \
		UTHASH_PACKAGE="$(UTHASH_PACKAGE)" \
		VERSION="$(VERSION)" \
		WANT_STRIPPED_BINARIES="$(WANT_STRIPPED_BINARIES)" \
		WINCC="$(WINCC)" \
		WINDRES="$(WINDRES)" \
		XFT_PACKAGE="$(XFT_PACKAGE)"
	$(hide) mv $@.tmp $@

clean:
	@echo CLEAN
	@rm -rf $(OBJDIR) bin

ifeq ($(LUA_INTERPRETER),$(OBJDIR)/lua)
$(LUA_INTERPRETER): src/c/emu/lua-5.1.5/*.[ch]
	@echo Bootstrapping build
	@mkdir -p $(dir $@)
	@$(CC) -o $(LUA_INTERPRETER) -O src/c/emu/lua-5.1.5/*.c src/c/emu/tmpnam.c -lm -DLUA_USE_EMU_TMPNAM
endif

.PHONY: distr
distr: wordgrinder-$(VERSION).tar.xz

.PHONY: debian-distr
debian-distr: wordgrinder-$(VERSION)-minimal-dependencies-for-debian.tar.xz

.PHONY: wordgrinder-$(VERSION).tar.xz
wordgrinder-$(VERSION).tar.xz:
	tar cvaf $@ \
		--transform "s,^,wordgrinder-$(VERSION)/," \
		extras \
		licenses \
		scripts \
		src \
		testdocs \
		tests \
		tools \
		build.lua \
		Makefile \
		README \
		README.wg \
		README.Windows.txt \
		wordgrinder.man \
		xwordgrinder.man

.PHONY: wordgrinder-$(VERSION)-minimal-dependencies-for-debian.tar.xz
wordgrinder-$(VERSION)-minimal-dependencies-for-debian.tar.xz:
	tar cvaf $@ \
		--transform "s,^,wordgrinder-$(VERSION)/," \
		--exclude "*.dictionary" \
		--exclude "src/c/emu" \
		extras \
		licenses \
		scripts \
		src \
		testdocs \
		tests \
		tools \
		build.lua \
		Makefile \
		README \
		README.wg \
		README.Windows.txt \
		wordgrinder.man \
		xwordgrinder.man

