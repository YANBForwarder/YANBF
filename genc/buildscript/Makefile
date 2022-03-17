#!/usr/bin/make -f

COMMON_PREFIX         := "$(CURDIR)/build"
COMMON_CONFIGURE      := ./configure CPPFLAGS="-I \"$(COMMON_PREFIX)/include\"" LDFLAGS="-L\"$(COMMON_PREFIX)/lib\"" --disable-shared --enable-static --prefix=$(COMMON_PREFIX)

libpng_PKG            := libpng-1.6.37.tar.xz
libpng_PATH           := libpng-1.6.37
libpng_CONFIGURE      := $(COMMON_CONFIGURE)

zlib_PKG              := zlib-1.2.11.tar.xz
zlib_PATH             := zlib-1.2.11
zlib_CONFIGURE        := ./configure --static --prefix=$(COMMON_PREFIX)

ImageMagick_PKG       := ImageMagick.tar.xz
ImageMagick_PATH      := ImageMagick-7.1.0-27
ImageMagick_CONFIGURE := $(COMMON_CONFIGURE) --disable-openmp --with-gslib=no --without-fontconfig --without-freetype --without-jpeg --without-lcms --without-openexr --without-pango --without-webp --without-x --without-xml

export LD_LIBRARY_PATH := "$(COMMON_PREFIX)/lib"
export PKG_CONFIG_PATH := "$(LD_LIBRARY_PATH)/pkgconfig"

.PHONY: yanbf build clean

yanbf: ImageMagick
	@cd .. && \
		./autogen.sh && \
		./configure PKG_CONFIG_PATH="$(PKG_CONFIG_PATH)" PKG_CONFIG="pkg-config --static" && \
		$(MAKE) LDFLAGS="-static -pthread" && \
		strip $@

clean:
	@$(RM) -r build/ \
		$(libpng_PATH) \
		$(zlib_PATH) \
		$(ImageMagick_PATH)

build:
	@[ -d $@ ] || mkdir $@

libpng: zlib
ImageMagick: libpng zlib

libpng zlib ImageMagick: %: | build
	@echo "Building $@"
	@[ -d $($@_PKG) ] || tar -xf $($@_PKG)
	@cd $($@_PATH) && \
		([ -f ./.configured ] || ($($@_CONFIGURE) && touch ./.configured)) && \
		([ -f ./.made ] || ($(MAKE) $($@_MAKE) && touch ./.made)) && \
		([ -f ./.installed ] || ($(MAKE) $($@_MAKE) install && touch ./.installed))
