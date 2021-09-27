#---------------------------------------------------------------------------------
# Goals for Build
#---------------------------------------------------------------------------------
.PHONY: all forwarder twlnfwd dist

all:	forwarder twlnfwd dist

dist:	forwarder twlnfwd
	@mkdir -p dist
	@cp -f forwarder/forwarder.* dist
	@cp -f twlnfwd/twlnfwd.cia dist

forwarder:
	@$(MAKE) -C forwarder

twlnfwd:
	@$(MAKE) -C twlnfwd
	make_cia --srl=twlnfwd/twlnfwd.nds

clean:
	@echo clean build directories
	@$(MAKE) -C forwarder clean
	@$(MAKE) -C twlnfwd clean
	@rm -rf dist