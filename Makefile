export PROJNAME := PCI
export RESULT := pci

all: $(RESULT)

%: force
	@$(MAKE) -f ../helper/Makefile $@ --no-print-directory
force: ;
