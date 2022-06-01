#include <cstdio>
#include "common.hpp"

extern "C" void _start() {
	if(std::getIO() != 0) {
		std::printf("PCI couldn't get I/O!");
		std::exit(1);
	}

	probe();

	std::halt();
}
