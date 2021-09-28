#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	bootstrap
export TOPDIR		:=	$(CURDIR)


.PHONY: checkarm7 checkarm9

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: bootstrap.cia

#---------------------------------------------------------------------------------
# Building separate .dsi and .nds binaries here is done purely to allow the .nds
# binary to be used to replace various flashcart boot menus.
#
# This is *NOT* for general use, standard .nds files with a full header are DSi
# hybrid binaries.
#
#---------------------------------------------------------------------------------
bootstrap.cia: bootstrap.dsi
	make_cia --srl=bootstrap.dsi

#---------------------------------------------------------------------------------
bootstrap.dsi : checkarm7 checkarm9
	ndstool -c $@ -7 $(TARGET).arm7.elf -9 $(TARGET).arm9i.elf -b $(CURDIR)/bootstrap.bmp "CTR-NDSForwarder;TWLNAND Booter;lifehackerhansol" -g TWLD 01 "TWLNFWDR-LHS" -u 00030005 -z 80040000

#---------------------------------------------------------------------------------
checkarm7:
	$(MAKE) -C arm7

#---------------------------------------------------------------------------------
checkarm9:
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
clean:
	$(MAKE) -C arm9 clean
	$(MAKE) -C arm7 clean
	rm -f $(TARGET).nds $(TARGET).arm7.elf $(TARGET).arm9i.elf bootstrap.dsi bootstrap.cia
