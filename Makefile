#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
.SECONDARY:

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

#---------------------------------------------------------------------------------
# Goals for Build
#---------------------------------------------------------------------------------
.PHONY: all forwarder bootstrap bootloader bootstub data dist

all:	forwarder bootstrap dist

dist:	forwarder bootstrap
	@mkdir -p dist/generator/data
	@mkdir -p dist/generator/romfs

	@cp -f bootstrap/bootstrap.cia 'dist/bootstrap.cia'
	@cp -f generator/data/build-cia.rsf dist/generator/data/build-cia.rsf
	@cp -f generator/data/dsboot.wav dist/generator/data/dsboot.wav
	@cp -f generator/*.py dist/generator
	@cp -f generator/requirements.txt dist/generator/requirements.txt
	@mv -f forwarder/forwarder.elf dist/generator/data/forwarder.elf
	@cp -f README.md dist/README.md

	@cd dist && zip -r ../YANBF.zip *

forwarder:
	@$(MAKE) -C forwarder

bootstrap: bootloader bootstub
	@$(MAKE) -C bootstrap

bootloader: data
	@$(MAKE) -C universal/bootloader LOADBIN=$(CURDIR)/universal/data/load.bin

bootstub: data
	@$(MAKE) -C universal/bootstub

data:
	@mkdir -p universal/data
clean:
	@echo clean build directories
	@$(MAKE) -C forwarder clean
	@$(MAKE) -C bootstrap clean
	@$(MAKE) -C universal/bootloader clean
	@$(MAKE) -C universal/bootstub clean
	@rm -rf dist universal/data YANBF.zip
