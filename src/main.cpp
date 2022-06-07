#include <cstdio>
#include "common.hpp"

std::unordered_map<uint16_t, std::vector<uint32_t>> classes;

extern "C" void _start() {
	if(std::getIO() != 0) {
		std::printf("PCI couldn't get I/O!");
		std::exit(1);
	}

	probe();

	// Reorganize the other map into classes
	for(auto const& x : descriptors) {
		uint16_t key = 0;
		key |= (uint16_t)x.s.common.classCode << 8;
		key |= x.s.common.subclassCode;

		classes[key].push_back(x.f);
	}

	exportProcedures();
	std::enableRPC();
	std::publish("PCI");
	std::halt();
}
