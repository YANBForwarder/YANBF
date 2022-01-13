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
	@mkdir -p 'dist/for SD card root/_nds/'
	@mkdir -p 'dist/for SD card root/cias'

	@cp -f forwarder/forwarder.* dist
	@cp -f bootstrap/bootstrap.cia 'dist/for SD card root/cias/bootstrap.cia'
	@cp -f generator/generator.py dist/generator/generator.py
	@cp -f generator/bannergif.py dist/generator/bannergif.py
	@cp -f generator/data/build-cia.rsf dist/generator/data/build-cia.rsf
	@mv -f dist/forwarder.elf dist/generator/data/forwarder.elf
	@rm -f dist/forwarder.*
	@cp -f README.md dist/README.md
	@cp -f generator/data/dsboot.wav dist/generator/data/dsboot.wav

	@cd dist && zip -r ../CTR-NDSForwarder.zip *

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
	@rm -rf dist universal/data CTR-NDSForwarder.zip
