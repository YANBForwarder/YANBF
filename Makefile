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

dist:	forwarder bootstrap sd
	@mkdir -p dist
	@cp -f forwarder/forwarder.* dist
	@cp -f bootstrap/bootstrap.cia dist
	@cp -f sd/sdcard.nds dist

forwarder:
	@$(MAKE) -C forwarder

bootstrap: bootloader
	@$(MAKE) -C bootstrap

sd: bootloader bootstub
	@$(MAKE) -C sd

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
	@$(MAKE) -C sd clean
	@$(MAKE) -C universal/bootloader clean
	@$(MAKE) -C universal/bootstub clean
	@rm -rf dist universal/data