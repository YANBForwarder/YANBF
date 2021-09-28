#---------------------------------------------------------------------------------
# Goals for Build
#---------------------------------------------------------------------------------
.PHONY: all forwarder bootstrap dist

all:	forwarder bootstrap dist

dist:	forwarder bootstrap
	@mkdir -p dist
	@cp -f forwarder/forwarder.* dist
	@cp -f bootstrap/bootstrap.cia dist

forwarder:
	@$(MAKE) -C forwarder

bootstrap:
	@$(MAKE) -C bootstrap
	make_cia --srl=bootstrap/bootstrap.nds

clean:
	@echo clean build directories
	@$(MAKE) -C forwarder clean
	@$(MAKE) -C bootstrap clean
	@rm -rf dist