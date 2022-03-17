# Makefile.am -- Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = tex3ds mkbcfnt

SOURCES = source/argparse.hpp \
          source/bannerpng.c \
          source/bannerpng.h \
          source/core.cpp \
          source/core.hpp \
          source/json.hpp \
          source/lodepng.c \
          source/lodepng.h \
          source/main.cpp \
          source/ndsheaderbanner.h

AM_CXXFLAGS = -I$(srcdir)/include -D_GNU_SOURCE $(ImageMagick_CFLAGS)

LDADD = $(ImageMagick_LIBS)

EXTRA_DIST = autogen.sh
